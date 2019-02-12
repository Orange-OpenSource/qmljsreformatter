/*
 * Copyright (C) 2017 Orange
 *
 * This software is distributed under the terms and conditions of the GNU
 * General Public Licence version 3 as published by the Free Software
 * Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-3.0.html.
 */

/*
 * Orange version of the Qt Creator's qmljsreformatter
 *
 * Create a qmljsreformatter binary from the modified sources of Qt Creator.
 *
 * Version:     1.0
 * Created:     2017-04-03 by Julien Déramond
 */

#include <QCommandLineParser>
#include <QDebug>
#include <QFile>

#include "src/qmljsreformatter.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("qmljsreformatter");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("QML JS reformatter");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", "Source file.");
    parser.addPositionalArgument("destination", "Destination file.");
    // A boolean option with multiple names (-s, --split)
    QCommandLineOption splitOption(QStringList() << "s" << "split", "Split the long lines.");
    parser.addOption(splitOption);
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.length() < 2) {
        qWarning() << "Usage:" << argv[0] << "<input-file> <output-file>";
        return 1;
    }

    QString inputFile = args.at(0);
    QString outputFile = args.at(1);

    QString content;

    QFile inFile(inputFile);
    if (inFile.open(QIODevice::ReadOnly)) {
        QTextStream ins(&inFile);
        content = ins.readAll();
        inFile.close();
    }
    else {
        qWarning() << "Error: couldn't open input file";
        return 2;
    }

    QmlJS::Document::MutablePtr doc = QmlJS::Document::create(inputFile, QmlJS::Dialect(QmlJS::Dialect::Qml));
    doc->setEditorRevision(0);
    doc->setSource(content);

    if (!doc->parse()) {
        qWarning() << "Error: doc->parse() execution";

        for (auto diagnosticMessage : doc->diagnosticMessages()) {
            qWarning("    (%d:%d) %s", diagnosticMessage.loc.startLine, diagnosticMessage.loc.startColumn, diagnosticMessage.message.toStdString().c_str());
        }

        return 3;
    }

    QString formattedContent = QmlJS::reformat(doc, parser.isSet(splitOption));
    QFile outFile(outputFile);

    if (outFile.open(QIODevice::WriteOnly)) {
        QTextStream outs(&outFile);
        outs << formattedContent;
        outFile.close();
    }
    else {
        qWarning() << "Error: couldn't open output file";
        return 4;
    }

    return 0;
}
