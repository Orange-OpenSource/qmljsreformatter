/*
 * Copyright (C) 2017 - 2019 Orange
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
 * Based on Qt Creator project. The modifications created by Orange in this
 * file:
 *    - implements the new signature of the 'reformat' function defined in the
 *      header file.
 *    - modify the original reformatting rules:
 *      - Force semicolon at the end of the JS statements.
 *      - Optional split line.
 *
 * Version:     1.0
 * Created:     2017-04-03 by Julien Déramond
 */

/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "qmljsreformatter.h"

#include "../qt-creator-simplified/src/libs/qmljs/qmljscodeformatter.h"
#include "../qt-creator-simplified/src/libs/qmljs/parser/qmljsast_p.h"
#include "../qt-creator-simplified/src/libs/qmljs/parser/qmljsastvisitor_p.h"
#include "../qt-creator-simplified/src/libs/qmljs/parser/qmljsengine_p.h"
#include "../qt-creator-simplified/src/libs/qmljs/parser/qmljslexer_p.h"

#include <QString>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextCursor>

#include <limits>

using namespace QmlJS;
using namespace QmlJS::AST;

namespace {

class SimpleFormatter : public QtStyleCodeFormatter
{
protected:
    class FormatterData : public QTextBlockUserData
    {
    public:
        FormatterData(const BlockData &data) : data(data) {}
        BlockData data;
    };

    void saveBlockData(QTextBlock *block, const BlockData &data) const override
    {
        block->setUserData(new FormatterData(data));
    }

    bool loadBlockData(const QTextBlock &block, BlockData *data) const override
    {
        if (!block.userData())
            return false;

        *data = static_cast<FormatterData *>(block.userData())->data;
        return true;
    }

    void saveLexerState(QTextBlock *block, int state) const override
    {
        block->setUserState(state);
    }

    int loadLexerState(const QTextBlock &block) const override
    {
        return block.userState();
    }
};

class Rewriter : protected Visitor
{
    Document::Ptr _doc;
    QString _result;
    QString _line;
    class Split {
    public:
        int offset;
        qreal badness;
    };
    QList<Split> _possibleSplits;
    QTextDocument _resultDocument;
    SimpleFormatter _formatter;
    int _indent = 0;
    int _nextComment = 0;
    int _lastNewlineOffset = -1;
    bool _hadEmptyLine = false;
    int _binaryExpDepth = 0;
    bool _hasOpenComment = false;
    bool _splitLongLines;

public:
    Rewriter(Document::Ptr doc, bool splitLongLines = false)
        : _doc(doc), _splitLongLines(splitLongLines)
    {
    }

    void setIndentSize(int size) { _formatter.setIndentSize(size); }
    void setTabSize(int size) { _formatter.setTabSize(size); }

    QString operator()(Node *node)
    {
        Q_ASSERT(node == _doc->ast()); // comment handling fails otherwise

        _result.reserve(_doc->source().size());
        _line.clear();
        _possibleSplits.clear();
        _indent = 0;
        _nextComment = 0;
        _lastNewlineOffset = -1;
        _hadEmptyLine = false;
        _binaryExpDepth = 0;

        // emit directives
        const QList<SourceLocation> &directives = _doc->jsDirectives();
        for (const auto &d: directives) {
            quint32 line = 1;
            int i = 0;
            while (line++ < d.startLine && i++ >= 0)
                i = _doc->source().indexOf(QChar('\n'), i);
            quint32 offset = static_cast<quint32>(i) + d.startColumn;
            int endline = _doc->source().indexOf('\n', static_cast<int>(offset) + 1);
            int end = endline == -1 ? _doc->source().length() : endline;
            quint32 length =  static_cast<quint32>(end) - offset;
            out(SourceLocation(offset, length, d.startLine, d.startColumn));
        }
        if (!directives.isEmpty())
            newLine();

        accept(node);

        // emit the final comments
        const QList<SourceLocation> &comments = _doc->engine()->comments();
        for (; _nextComment < comments.size(); ++_nextComment) {
            outComment(comments.at(_nextComment));
        }

        // ensure good ending
        if (!_result.endsWith(QLatin1String("\n\n")) || !_line.isEmpty())
            newLine();

        return _result;
    }

protected:
    void accept(Node *node)
    {
        Node::accept(node, this);
    }

    void lnAcceptIndented(Node *node)
    {
        newLine();
        accept(node);
    }

    void out(const char *str, const SourceLocation &lastLoc = SourceLocation())
    {
        out(QString::fromLatin1(str), lastLoc);
    }

    void outCommentText(const QString &str)
    {
        QStringList lines = str.split(QLatin1Char('\n'));
        bool multiline = lines.length() > 1;
        for (int i = 0; i < lines.size(); ++i) {
            if (multiline) {
                if (i == 0)
                    newLine();
                _line = lines.at(i);  // multiline comments don't keep track of previos lines
            }
            else
                _line += lines.at(i);
            if (i != lines.size() - 1)
                newLine();
        }
        _hadEmptyLine = false;
    }

    void outComment(const SourceLocation &commentLoc)
    {
        SourceLocation fixedLoc = commentLoc;
        fixCommentLocation(fixedLoc);
        if (precededByEmptyLine(fixedLoc) && !_result.endsWith(QLatin1String("\n\n")))
            newLine();
        outCommentText(toString(fixedLoc)); // don't use the sourceloc overload here
        if (followedByNewLine(fixedLoc))
            newLine();
        else
            out(" ");
    }

    void out(const QString &str, const SourceLocation &lastLoc = SourceLocation())
    {
        //qDebug() << "##out(QString str,lastLoc)##" << str;
        if (_hasOpenComment) {
            newLine();
        }
        if (lastLoc.isValid()) {
            QList<SourceLocation> comments = _doc->engine()->comments();
            for (; _nextComment < comments.size(); ++_nextComment) {
                SourceLocation commentLoc = comments.at(_nextComment);
                if (commentLoc.end() > lastLoc.end())
                    break;

                outComment(commentLoc);
            }
        }

        QStringList lines = str.split(QLatin1Char('\n'));
        for (int i = 0; i < lines.size(); ++i) {
            _line += lines.at(i);
            if (i != lines.size() - 1)
                newLine();
        }
        _hadEmptyLine = false;
    }

    QString toString(const SourceLocation &loc)
    {
        return _doc->source().mid(loc.offset, loc.length);
    }

    void out(const SourceLocation &loc)
    {
        if (!loc.isValid())
            return;
        out(toString(loc), loc);
    }

    int tryIndent(const QString &line)
    {
        // append the line to the text document
        QTextCursor cursor(&_resultDocument);
        cursor.movePosition(QTextCursor::End);
        int cursorStartLinePos = cursor.position();
        cursor.insertText(line);

        // get the expected indentation
        QTextBlock last = _resultDocument.lastBlock();
        _formatter.updateStateUntil(last);
        int indent = _formatter.indentFor(last);

        // remove the line again
        cursor.setPosition(cursorStartLinePos);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

        return indent;
    }

    void finishLine()
    {
        // remove trailing spaces
        int len = _line.size();
        while (len > 0 && _line.at(len - 1).isSpace())
            --len;
        _line.resize(len);

        _line += QLatin1Char('\n');

        _result += _line;
        QTextCursor cursor(&_resultDocument);
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(_line);

        _line = QString(_indent, QLatin1Char(' '));
    }

    class BestSplit {
    public:
        QStringList lines;
        qreal badnessFromSplits;

        qreal badness()
        {
            const int maxLineLength = 80;
            const int strongMaxLineLength = 100;
            const int minContentLength = 10;

            qreal result = badnessFromSplits;
            foreach (const QString &line, lines) {
                // really long lines should be avoided at all cost
                if (line.size() > strongMaxLineLength) {
                    result += 50 + (line.size() - strongMaxLineLength);
                // having long lines is bad
                } else if (line.size() > maxLineLength) {
                    result += 3 + (line.size() - maxLineLength);
                // and even ok-sized lines should have a cost
                } else {
                    result += 1;
                }

                // having lines with little content is bad
                const int contentSize = line.trimmed().size();
                if (contentSize < minContentLength)
                    result += 0.5;
            }
            return result;
        }
    };

    void adjustIndent(QString *line, QList<Split> *splits, int indent)
    {
        int startSpaces = 0;
        while (startSpaces < line->size() && line->at(startSpaces).isSpace())
            ++startSpaces;

        line->replace(0, startSpaces, QString(indent, QLatin1Char(' ')));
        for (int i = 0; i < splits->size(); ++i) {
            (*splits)[i].offset = splits->at(i).offset - startSpaces + indent;
        }
    }

    BestSplit computeBestSplits(QStringList context, QString line, QList<Split> possibleSplits)
    {
        BestSplit result;

        while (possibleSplits.count() > 12) {
             QList<Split> newPossibleSplits;
             for (int i = 0; i < possibleSplits.count(); i++) {
                 if (!(i % 2))
                     newPossibleSplits.push_back(possibleSplits.at(i));
             }
             possibleSplits = newPossibleSplits;
        }

        result.badnessFromSplits = 0;
        result.lines = QStringList(line);

        // try to split at every possible position
        for (int i = possibleSplits.size() - 1; i >= 0; --i) {
            const int splitPos = possibleSplits.at(i).offset;
            const QString newContextLine = line.left(splitPos);
            QStringList newContext = context;
            newContext += newContextLine;
            const QString restLine = line.mid(splitPos);
            if (restLine.trimmed().isEmpty())
                continue;

            // the extra space is to avoid // comments sticking to the 0 offset
            QString indentLine = newContext.join(QLatin1Char('\n')) + QLatin1String("\n ") + restLine;
            int indent = tryIndent(indentLine);

            QList<Split> newSplits = possibleSplits.mid(i + 1);
            QString newSplitLine = restLine;
            adjustIndent(&newSplitLine, &newSplits, indent);

            for (int j = 0; j < newSplits.size(); ++j)
                newSplits[j].offset = newSplits.at(j).offset - splitPos;

            BestSplit nested = computeBestSplits(newContext, newSplitLine, newSplits);

            nested.lines.prepend(newContextLine);
            nested.badnessFromSplits += possibleSplits.at(i).badness;
            if (nested.badness() < result.badness())
                result = nested;
        }

        return result;
    }

    void newLine()
    {
        // if preceded by a newline, it's an empty line!
        _hadEmptyLine = _line.trimmed().isEmpty();
        _hasOpenComment = false;

        // if the preceding line wasn't empty, reindent etc.
        if (!_hadEmptyLine) {
            int indentStart = 0;
            while (indentStart < _line.size() && _line.at(indentStart).isSpace())
                ++indentStart;

            _indent = tryIndent(_line);
            adjustIndent(&_line, &_possibleSplits, _indent);

            // maybe make multi-line?
            if (_splitLongLines) {
                BestSplit split = computeBestSplits(QStringList(), _line, _possibleSplits);
                if (!split.lines.isEmpty() && split.lines.size() > 1) {
                    for (int i = 0; i < split.lines.size(); ++i) {
                        _line = split.lines.at(i);
                        if (i != split.lines.size() - 1)
                            finishLine();
                    }
                }
            }
        }

        finishLine();
        _possibleSplits.clear();
    }

    void requireEmptyLine()
    {
        while (!_hadEmptyLine)
            newLine();
    }

    bool acceptBlockOrIndented(Node *ast)
    {
        if (cast<Block *>(ast)) {
            out(" ");
            accept(ast);
            return true;
        } else {
            out(" {");
            lnAcceptIndented(ast);
            newLine();
            out("}");
            return false;
        }
    }

    bool followedByNewLine(const SourceLocation &loc)
    {
        const QString &source = _doc->source();
        int i = loc.end();
        for (; i < source.size() && source.at(i).isSpace(); ++i) {
            if (source.at(i) == QLatin1Char('\n'))
                return true;
        }
        return false;
    }

    bool precededByEmptyLine(const SourceLocation &loc)
    {
        const QString &source = _doc->source();
        int i = loc.offset;

        // expect spaces and \n, twice
        bool first = true;
        for (--i;
             i >= 0 && source.at(i).isSpace();
             --i) {

            if (source.at(i) == QLatin1Char('\n')) {
                if (first)
                    first = false;
                else
                    return true;
            }
        }
        return false;
    }

    void addPossibleSplit(qreal badness, int offset = 0)
    {
        Split s;
        s.badness = badness;
        s.offset = _line.size() + offset;
        _possibleSplits += s;
    }

    void fixCommentLocation(SourceLocation &loc)
    {
        loc.offset -= 2;
        loc.startColumn -= 2;
        loc.length += 2;
        if (_doc->source().mid(loc.offset, 2) == QLatin1String("/*"))
            loc.length += 2;
    }

    bool preVisit(Node *ast) override
    {
        semicolonsAlgorithm(ast, true);

        SourceLocation firstLoc;
        if (ExpressionNode *expr = ast->expressionCast())
            firstLoc = expr->firstSourceLocation();
        else if (Statement *stmt = ast->statementCast())
            firstLoc = stmt->firstSourceLocation();
        else if (UiObjectMember *mem = ast->uiObjectMemberCast())
            firstLoc = mem->firstSourceLocation();
        else if (UiImport *import = cast<UiImport *>(ast))
            firstLoc = import->firstSourceLocation();

        if (firstLoc.isValid() && int(firstLoc.offset) != _lastNewlineOffset) {
            _lastNewlineOffset = firstLoc.offset;

            if (precededByEmptyLine(firstLoc) && !_result.endsWith(QLatin1String("\n\n")))
                newLine();
        }

        return true;
    }

    void postVisit(Node *ast) override
    {
        semicolonsAlgorithm(ast, false);

        SourceLocation lastLoc;
        if (ExpressionNode *expr = ast->expressionCast())
            lastLoc = expr->lastSourceLocation();
        else if (Statement *stmt = ast->statementCast())
            lastLoc = stmt->lastSourceLocation();
        else if (UiObjectMember *mem = ast->uiObjectMemberCast())
            lastLoc = mem->lastSourceLocation();
        else if (UiImport *import = cast<UiImport *>(ast))
            lastLoc = import->lastSourceLocation();

        if (lastLoc.isValid()) {
            const QList<SourceLocation> &comments = _doc->engine()->comments();

            // preserve trailing comments
            for (; _nextComment < comments.size(); ++_nextComment) {
                SourceLocation nextCommentLoc = comments.at(_nextComment);
                if (nextCommentLoc.startLine != lastLoc.startLine)
                    break;
                fixCommentLocation(nextCommentLoc);

                // there must only be whitespace between lastLoc and the comment
                bool commentFollows = true;
                for (quint32 i = lastLoc.end(); i < nextCommentLoc.begin(); ++i) {
                    if (!_doc->source().at(i).isSpace()) {
                        commentFollows = false;
                        break;
                    }
                }
                if (!commentFollows)
                    break;

                out(" ");
                out(toString(nextCommentLoc));
                _hasOpenComment = true;
            }
        }
    }

    bool visit(UiPragma *ast) override
    {
        out("pragma ", ast->pragmaToken);
        out(ast->name.toString());
        newLine();
        return false;
    }

    bool visit(UiEnumDeclaration *ast) override
    {
        out(ast->enumToken);
        out(" ");
        out(ast->name.toString());
        out(" ");
        out("{"); // TODO: out(ast->lbraceToken);
        newLine();

        accept(ast->members);

        out(ast->rbraceToken);
        return false;
    }

    bool visit(UiEnumMemberList *list) override
    {
        for (UiEnumMemberList *it = list; it; it = it->next) {
            out(it->memberToken);
            if (it->valueToken.isValid()) {
                out(" = ");
                out(it->valueToken);
            }
            if (it->next) {
                out(",");
            }
            newLine();
        }
        return false;
    }
       

    bool visit(UiImport *ast) override
    {
        out("import ", ast->importToken);
        if (!ast->fileName.isNull())
            out(QString::fromLatin1("\"%1\"").arg(ast->fileName.toString()));
        else
            accept(ast->importUri);
        if (ast->versionToken.isValid()) {
            out(" ");
            out(ast->versionToken);
        }
        if (!ast->importId.isNull()) {
            out(" as ", ast->asToken);
            out(ast->importIdToken);
        }
        return false;
    }

    bool visit(UiObjectDefinition *ast) override
    {
        accept(ast->qualifiedTypeNameId);
        out(" ");
        accept(ast->initializer);
        return false;
    }

    bool visit(UiObjectInitializer *ast) override
    {
        out(ast->lbraceToken);
        if (ast->members) {
            lnAcceptIndented(ast->members);
            newLine();
        }
        out(ast->rbraceToken);
        return false;
    }

    bool visit(UiParameterList *list) override
    {
        for (UiParameterList *it = list; it; it = it->next) {
            out(it->propertyTypeToken);
            out(" ");
            out(it->identifierToken);
            if (it->next)
                out(", ", it->commaToken);
        }
        return false;
    }

    bool visit(UiPublicMember *ast) override
    {
        if (ast->type == UiPublicMember::Property) {
            if (ast->isDefaultMember)
                out("default ", ast->defaultToken);
            else if (ast->isReadonlyMember)
                out("readonly ", ast->readonlyToken);
            out("property ", ast->propertyToken);
            if (!ast->typeModifier.isNull()) {
                out(ast->typeModifierToken);
                out("<");
                accept(ast->memberType);
                out(">");
            } else {
                accept(ast->memberType);
            }
            out(" ");
            if (ast->statement) {
                out(ast->identifierToken);
                out(": ", ast->colonToken);
                accept(ast->statement);
            } else if (ast->binding) {
                accept(ast->binding);
            } else {
                out(ast->identifierToken);
            }
        } else { // signal
            out("signal ", ast->identifierToken);
            out(ast->identifierToken);
            if (ast->parameters) {
                out("(");
                accept(ast->parameters);
                out(")");
            }
        }
        return false;
    }

    bool visit(UiObjectBinding *ast) override
    {
        if (ast->hasOnToken) {
            accept(ast->qualifiedTypeNameId);
            out(" on ");
            accept(ast->qualifiedId);
        } else {
            accept(ast->qualifiedId);
            out(": ", ast->colonToken);
            accept(ast->qualifiedTypeNameId);
        }
        out(" ");
        accept(ast->initializer);
        return false;
    }

    bool visit(UiScriptBinding *ast) override
    {
        accept(ast->qualifiedId);
        out(": ", ast->colonToken);
        accept(ast->statement);
        return false;
    }

    bool visit(UiArrayBinding *ast) override
    {
        accept(ast->qualifiedId);
        out(ast->colonToken);
        out(" ");
        out(ast->lbracketToken);
        lnAcceptIndented(ast->members);
        newLine();
        out(ast->rbracketToken);
        return false;
    }

    bool visit(ThisExpression *ast) override { out(ast->thisToken); return true; }
    bool visit(NullExpression *ast) override { out(ast->nullToken); return true; }
    bool visit(TrueLiteral *ast) override { out(ast->trueToken); return true; }
    bool visit(FalseLiteral *ast) override { out(ast->falseToken); return true; }
    bool visit(IdentifierExpression *ast) override { out(ast->identifierToken); return true; }
    bool visit(StringLiteral *ast) override { out(ast->literalToken); return true; }
    bool visit(NumericLiteral *ast) override { out(ast->literalToken); return true; }
    bool visit(RegExpLiteral *ast) override { out(ast->literalToken); return true; }

    bool visit(ArrayPattern *ast) override
    {
        out(ast->lbracketToken);
        if (ast->elements)
            accept(ast->elements);
        out(ast->rbracketToken);
        return false;
    }

    bool visit(ObjectPattern *ast) override
    {
        out(ast->lbraceToken);
        if (ast->properties) {
            lnAcceptIndented(ast->properties);
            newLine();
        }
        out(ast->rbraceToken);
        return false;
    }

    bool visit(PatternElementList *ast) override
    {
        for (PatternElementList *it = ast; it; it = it->next) {
            if (it->elision)
                accept(it->elision);
            if (it->elision && it->element)
                out(", ");
            if (it->element)
                accept(it->element);
            if (it->next)
                out(", ");
        }
        return false;
    }

    bool visit(PatternPropertyList *ast) override
    {
        for (PatternPropertyList *it = ast; it; it = it->next) {
            PatternProperty *assignment = AST::cast<PatternProperty *>(it->property);
            if (assignment) {
                out("\"");
                accept(assignment->name);
                out("\"");
                out(": ", assignment->colonToken);
                accept(assignment->initializer);
                if (it->next) {
                    out(","); // always invalid?
                    newLine();
                }
                continue;
            }
            PatternPropertyList *getterSetter = AST::cast<PatternPropertyList *>(it->next);
            if (getterSetter->property) {
                switch (getterSetter->property->type) {
                case PatternElement::Getter:
                    out("get");
                    break;
                case PatternElement::Setter:
                    out("set");
                    break;
                default:
                    break;
                }

                accept(getterSetter->property->name);
                out("(");
                //accept(getterSetter->formals);  // TODO
                out(")");
                out(" {");
                //accept(getterSetter->functionBody);  // TODO
                out(" }");
            }
        }
        return false;
    }

    bool visit(NestedExpression *ast) override
    {
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        return false;
    }

    bool visit(IdentifierPropertyName *ast) override { out(ast->id.toString()); return true; }
    bool visit(StringLiteralPropertyName *ast) override { out(ast->id.toString()); return true; }
    bool visit(NumericLiteralPropertyName *ast) override { out(QString::number(ast->id)); return true; }

    bool visit(ArrayMemberExpression *ast) override
    {
        accept(ast->base);
        out(ast->lbracketToken);
        accept(ast->expression);
        out(ast->rbracketToken);
        return false;
    }

    bool visit(FieldMemberExpression *ast) override
    {
        accept(ast->base);
        out(ast->dotToken);
        out(ast->identifierToken);
        return false;
    }

    bool visit(NewMemberExpression *ast) override
    {
        out("new ", ast->newToken);
        accept(ast->base);
        out(ast->lparenToken);
        accept(ast->arguments);
        out(ast->rparenToken);
        return false;
    }

    bool visit(NewExpression *ast) override
    {
        out("new ", ast->newToken);
        accept(ast->expression);
        return false;
    }

    bool visit(CallExpression *ast) override
    {
        accept(ast->base);
        out(ast->lparenToken);
        addPossibleSplit(0);
        accept(ast->arguments);
        out(ast->rparenToken);
        return false;
    }

    bool visit(PostIncrementExpression *ast) override
    {
        accept(ast->base);
        out(ast->incrementToken);
        return false;
    }

    bool visit(PostDecrementExpression *ast) override
    {
        accept(ast->base);
        out(ast->decrementToken);
        return false;
    }

    bool visit(PreIncrementExpression *ast) override
    {
        out(ast->incrementToken);
        accept(ast->expression);
        return false;
    }

    bool visit(PreDecrementExpression *ast) override
    {
        out(ast->decrementToken);
        accept(ast->expression);
        return false;
    }

    bool visit(DeleteExpression *ast) override
    {
        out("delete ", ast->deleteToken);
        accept(ast->expression);
        return false;
    }

    bool visit(VoidExpression *ast) override
    {
        out("void ", ast->voidToken);
        accept(ast->expression);
        return false;
    }

    bool visit(TypeOfExpression *ast) override
    {
        out("typeof ", ast->typeofToken);
        accept(ast->expression);
        return false;
    }

    bool visit(UnaryPlusExpression *ast) override
    {
        out(ast->plusToken);
        accept(ast->expression);
        return false;
    }

    bool visit(UnaryMinusExpression *ast) override
    {
        out(ast->minusToken);
        accept(ast->expression);
        return false;
    }

    bool visit(TildeExpression *ast) override
    {
        out(ast->tildeToken);
        accept(ast->expression);
        return false;
    }

    bool visit(NotExpression *ast) override
    {
        out(ast->notToken);
        accept(ast->expression);
        return false;
    }

    bool visit(BinaryExpression *ast) override
    {
        ++_binaryExpDepth;
        accept(ast->left);

        // in general, avoid splitting at the operator
        // but && and || are ok
        qreal splitBadness = 30;
        if (ast->op == QSOperator::And
                || ast->op == QSOperator::Or)
            splitBadness = 0;
        addPossibleSplit(splitBadness);

        out(" ");
        out(ast->operatorToken);
        out(" ");
        accept(ast->right);
        --_binaryExpDepth;
        return false;
    }

    bool visit(ConditionalExpression *ast) override
    {
        accept(ast->expression);
        out(" ? ", ast->questionToken);
        accept(ast->ok);
        out(" : ", ast->colonToken);
        accept(ast->ko);
        return false;
    }

    bool visit(Block *ast) override
    {
        out(ast->lbraceToken);
        lnAcceptIndented(ast->statements);
        newLine();
        out(ast->rbraceToken);
        return false;
    }

    bool visit(VariableStatement *ast) override
    {
        out(ast->declarationKindToken);
        out(" ");
        accept(ast->declarations);
        return false;
    }

    bool visit(PatternElement *ast) override
    {
        if (ast->isForDeclaration) {
            if (ast->scope == VariableScope::Var) {
                out("var ");
            } else if (ast->scope == VariableScope::Let) {
                out("let ");
            } else if (ast->scope == VariableScope::Const) {
                out("const ");
            }
        }
        out(ast->identifierToken);
        if (ast->initializer) {
            if (ast->isVariableDeclaration())
                out(" = ");
            accept(ast->initializer);
        }
        return false;
    }

    bool visit(EmptyStatement *ast) override
    {
        out(ast->semicolonToken);
        return false;
    }

    bool visit(IfStatement *ast) override
    {
        out(ast->ifToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        acceptBlockOrIndented(ast->ok);
        if (ast->ko) {
            newLine();
            out(ast->elseToken);
            if (cast<Block *>(ast->ko) || cast<IfStatement *>(ast->ko)) {
                out(" ");
                accept(ast->ko);
            } else {
                out(" {");
                lnAcceptIndented(ast->ko);
                newLine();
                out("}");
            }
        }
        return false;
    }

    bool visit(DoWhileStatement *ast) override
    {
        out(ast->doToken);
        acceptBlockOrIndented(ast->statement);
        newLine();
        out(ast->whileToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        return false;
    }

    bool visit(WhileStatement *ast) override
    {
        out(ast->whileToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        acceptBlockOrIndented(ast->statement);
        return false;
    }

    bool visit(ForStatement *ast) override
    {
        out(ast->forToken);
        out(" ");
        out(ast->lparenToken);
        if (ast->initialiser) {
            accept(ast->initialiser);
        } else if (ast->declarations) {
            out("var ");
            accept(ast->declarations);
        }
        out("; ", ast->firstSemicolonToken);
        accept(ast->condition);
        out("; ", ast->secondSemicolonToken);
        accept(ast->expression);
        out(ast->rparenToken);
        acceptBlockOrIndented(ast->statement);
        return false;
    }

    bool visit(ForEachStatement *ast) override
    {
        out(ast->forToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->lhs);
        out(" in ");
        accept(ast->expression);
        out(ast->rparenToken);
        acceptBlockOrIndented(ast->statement);
        return false;
    }

    bool visit(ContinueStatement *ast) override
    {
        out(ast->continueToken);
        if (!ast->label.isNull()) {
            out(" ");
            out(ast->identifierToken);
        }
        return false;
    }

    bool visit(BreakStatement *ast) override
    {
        out(ast->breakToken);
        if (!ast->label.isNull()) {
            out(" ");
            out(ast->identifierToken);
        }
        return false;
    }

    bool visit(ReturnStatement *ast) override
    {
        out(ast->returnToken);
        if (ast->expression) {
            out(" ");
            accept(ast->expression);
        }
        return false;
    }

    bool visit(ThrowStatement *ast) override
    {
        out(ast->throwToken);
        if (ast->expression) {
            out(" ");
            accept(ast->expression);
        }
        return false;
    }

    bool visit(WithStatement *ast) override
    {
        out(ast->withToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        acceptBlockOrIndented(ast->statement);
        return false;
    }

    bool visit(SwitchStatement *ast) override
    {
        out(ast->switchToken);
        out(" ");
        out(ast->lparenToken);
        accept(ast->expression);
        out(ast->rparenToken);
        out(" ");
        accept(ast->block);
        return false;
    }

    bool visit(CaseBlock *ast) override
    {
        out(ast->lbraceToken);
        newLine();
        accept(ast->clauses);
        if (ast->clauses && ast->defaultClause)
            newLine();
        accept(ast->defaultClause);
        if (ast->moreClauses)
            newLine();
        accept(ast->moreClauses);
        newLine();
        out(ast->rbraceToken);
        return false;
    }

    bool visit(CaseClause *ast) override
    {
        out("case ", ast->caseToken);
        accept(ast->expression);
        out(ast->colonToken);
        if (ast->statements)
            lnAcceptIndented(ast->statements);
        return false;
    }

    bool visit(DefaultClause *ast) override
    {
        out(ast->defaultToken);
        out(ast->colonToken);
        lnAcceptIndented(ast->statements);
        return false;
    }

    bool visit(LabelledStatement *ast) override
    {
        out(ast->identifierToken);
        out(": ", ast->colonToken);
        accept(ast->statement);
        return false;
    }

    bool visit(TryStatement *ast) override
    {
        out("try ", ast->tryToken);
        accept(ast->statement);
        if (ast->catchExpression) {
            out(" ");
            accept(ast->catchExpression);
        }
        if (ast->finallyExpression) {
            out(" ");
            accept(ast->finallyExpression);
        }
        return false;
    }

    bool visit(Catch *ast) override
    {
        out(ast->catchToken);
        out(" ");
        out(ast->lparenToken);
        out(ast->identifierToken);
        out(") ", ast->rparenToken);
        accept(ast->statement);
        return false;
    }

    bool visit(Finally *ast) override
    {
        out("finally ", ast->finallyToken);
        accept(ast->statement);
        return false;
    }

    bool visit(FunctionDeclaration *ast) override
    {
        return visit(static_cast<FunctionExpression *>(ast));
    }

    bool visit(FunctionExpression *ast) override
    {
        out("function ", ast->functionToken);
        if (!ast->name.isNull())
            out(ast->identifierToken);
        out(ast->lparenToken);
        accept(ast->formals);
        out(ast->rparenToken);
        out(" ");
        out(ast->lbraceToken);
        if (ast->body) {
            lnAcceptIndented(ast->body);
            newLine();
        }
        out(ast->rbraceToken);
        return false;
    }

    bool visit(UiHeaderItemList *ast) override
    {
        for (UiHeaderItemList *it = ast; it; it = it->next) {
            accept(it->headerItem);
            newLine();
        }
        requireEmptyLine();
        return false;
    }

    bool visit(UiObjectMemberList *ast) override
    {
        for (UiObjectMemberList *it = ast; it; it = it->next) {
            accept(it->member);
            if (it->next)
                newLine();
        }
        return false;
    }

    bool visit(UiArrayMemberList *ast) override
    {
        for (UiArrayMemberList *it = ast; it; it = it->next) {
            accept(it->member);
            if (it->next) {
                out(",", ast->commaToken); // ### comma token seems to be generally invalid
                newLine();
            }
        }
        return false;
    }

    bool visit(UiQualifiedId *ast) override
    {
        for (UiQualifiedId *it = ast; it; it = it->next) {
            out(it->identifierToken);
            if (it->next)
                out(".");
        }
        return false;
    }

    bool visit(Elision *ast) override
    {
        for (Elision *it = ast; it; it = it->next) {
            if (it->next)
                out(", ", ast->commaToken);
        }
        return false;
    }

    bool visit(ArgumentList *ast) override
    {
        for (ArgumentList *it = ast; it; it = it->next) {
            accept(it->expression);
            if (it->next) {
                out(", ", it->commaToken);
                addPossibleSplit(-1);
            }
        }
        return false;
    }

    bool visit(StatementList *ast) override
    {
        for (StatementList *it = ast; it; it = it->next) {
            // ### work around parser bug: skip empty statements with wrong tokens
            if (EmptyStatement *emptyStatement = cast<EmptyStatement *>(it->statement)) {
                if (toString(emptyStatement->semicolonToken) != QLatin1String(";"))
                    continue;
            }

            accept(it->statement);
            if (it->next)
                newLine();
        }
        return false;
    }

    bool visit(VariableDeclarationList *ast) override
    {
        for (VariableDeclarationList *it = ast; it; it = it->next) {
            accept(it->declaration);
            if (it->next)
                out(", ", it->commaToken);
        }
        return false;
    }

    bool visit(CaseClauses *ast) override
    {
        for (CaseClauses *it = ast; it; it = it->next) {
            accept(it->clause);
            if (it->next)
                newLine();
        }
        return false;
    }

    bool visit(FormalParameterList *ast) override
    {
        for (FormalParameterList *it = ast; it; it = it->next) {
            out(it->element->bindingIdentifier.toString()); // TODO
            if (it->next)
                out(", ");
        }
        return false;
    }

    /****************************************************************************
     **
     ** Semicolons algorithm - Begin
     **
     ****************************************************************************/
    QVector<QVector<bool> > QMLTreeNode;
    QVector<QString> QMLTreeNodeType;
    bool successivePreVisits = false;
    int tab = 0;

    void addSimpleNode(bool canAddSemicolons) {
        QMLTreeNodeType.push_back("");
        QMLTreeNode.push_back({canAddSemicolons});
    }

    void addComplexNode(bool canAddSemicolons, QVector<bool> canChildrenAddSemicolonsVector) {
        addSimpleNode(canAddSemicolons);
        QMLTreeNodeType.push_back("node");
        QMLTreeNode.push_back(canChildrenAddSemicolonsVector);
    }

    void consumeNode(bool mustHandleSemicolons = false) {
        if (mustHandleSemicolons) {
            handleSemicolons();
        }

        // Re-establish the previous node if there's one.
        if (QMLTreeNodeType.size() > 1) {
            if (QMLTreeNodeType.at(QMLTreeNodeType.size() - 2) == "node") {
                QMLTreeNode[QMLTreeNode.size()-2].remove(0);
            }
            if (QMLTreeNode[QMLTreeNode.size()-2].size() == 0) {
                QMLTreeNode.remove(QMLTreeNode.size()-2);
                QMLTreeNodeType.remove(QMLTreeNode.size()-2);
            }
        }

        // Take care of the current node.
        if ((QMLTreeNode.size() > 0)) {
            if (QMLTreeNode[QMLTreeNode.size()-1].size() > 0) {
                QMLTreeNode[QMLTreeNode.size()-1].remove(0);
            }
            if (QMLTreeNode[QMLTreeNode.size()-1].size() == 0) {
                QMLTreeNode.pop_back();
                QMLTreeNodeType.pop_back();
            }
        }
    }

    void handleSemicolons() {
        if (QMLTreeNode.size() > 1 && QMLTreeNode.at(QMLTreeNode.size() - 2).size() > 0 && QMLTreeNode.at(QMLTreeNode.size() - 2).at(0)) {
            out(";");
        }
    }

    bool isPreviousNodeEligible() {
        return (QMLTreeNode.size() > 0 && QMLTreeNode.at(QMLTreeNode.size() - 1).size() > 0 && QMLTreeNode.at(QMLTreeNode.size() - 1).at(0));
    }

    QString displayTag(QString tagName, bool openingTag) {
        return (openingTag ? "<" : "</") + tagName;
    }

    void semicolonsAlgorithm(Node *ast, bool preVisit) {
        QString display;

        // Format the tree with tabulations.
        if (preVisit && successivePreVisits) {
            tab++;
        }
        else if (!preVisit && !successivePreVisits) {
            tab--;
        }
        successivePreVisits = preVisit;

        for (int i = 0 ; i < tab ; i++) {
            display += "    ";
        }

        // Detect and display the type of element
        if (cast<ArgumentList*>(ast)) display += displayTag("ArgumentList", preVisit);
        else if (cast<ArrayMemberExpression*>(ast)) display += displayTag("ArrayMemberExpression", preVisit);
        else if (cast<ArrayPattern*>(ast)) display += displayTag("ArrayPattern", preVisit);
        else if (cast<BinaryExpression*>(ast)) display += displayTag("BinaryExpression", preVisit);
        else if (cast<Block*>(ast)) display += displayTag("Block", preVisit);
        else if (cast<BreakStatement*>(ast)) display += displayTag("BreakStatement", preVisit);
        else if (cast<CallExpression*>(ast)) display += displayTag("CallExpression", preVisit);
        else if (cast<CaseBlock*>(ast)) display += displayTag("CaseBlock", preVisit);
        else if (cast<CaseClause*>(ast)) display += displayTag("CaseClause", preVisit);
        else if (cast<CaseClauses*>(ast)) display += displayTag("CaseClauses", preVisit);
        else if (cast<Catch*>(ast)) display += displayTag("Catch", preVisit);
        else if (cast<ConditionalExpression*>(ast)) display += displayTag("ConditionalExpression", preVisit);
        else if (cast<ContinueStatement*>(ast)) display += displayTag("ContinueStatement", preVisit);
        else if (cast<DebuggerStatement*>(ast)) display += displayTag("DebuggerStatement [WARNING: Not yet taken into account]", preVisit);
        else if (cast<DefaultClause*>(ast)) display += displayTag("DefaultClause", preVisit);
        else if (cast<DeleteExpression*>(ast)) display += displayTag("DeleteExpression", preVisit);
        else if (cast<DoWhileStatement*>(ast)) display += displayTag("DoWhileStatement", preVisit);
        else if (cast<Elision*>(ast)) display += displayTag("Elision", preVisit);
        else if (cast<EmptyStatement*>(ast)) display += displayTag("EmptyStatement", preVisit);
        else if (cast<Expression*>(ast)) display += displayTag("Expression", preVisit);
        else if (cast<ExpressionStatement*>(ast)) display += displayTag("ExpressionStatement", preVisit);
        else if (cast<FalseLiteral*>(ast)) display += displayTag("FalseLiteral", preVisit);
        else if (cast<FieldMemberExpression*>(ast)) display += displayTag("FieldMemberExpression", preVisit);
        else if (cast<Finally*>(ast)) display += displayTag("Finally", preVisit);
        else if (cast<ForEachStatement*>(ast)) display += displayTag("ForEachStatement", preVisit);
        else if (cast<FormalParameterList*>(ast)) display += displayTag("FormalParameterList", preVisit);
        else if (cast<ForStatement*>(ast)) display += displayTag("ForStatement", preVisit);
        else if (cast<FunctionDeclaration*>(ast)) display += displayTag("FunctionDeclaration", preVisit);
        else if (cast<FunctionExpression*>(ast)) display += displayTag("FunctionExpression", preVisit);
        else if (cast<IdentifierExpression*>(ast)) display += displayTag("IdentifierExpression", preVisit);
        else if (cast<IdentifierPropertyName*>(ast)) display += displayTag("IdentifierPropertyName", preVisit);
        else if (cast<IfStatement*>(ast)) display += displayTag("IfStatement", preVisit);
        else if (cast<LabelledStatement*>(ast)) display += displayTag("LabelledStatement", preVisit);
        else if (cast<NestedExpression*>(ast)) display += displayTag("NestedExpression", preVisit);
        else if (cast<NewExpression*>(ast)) display += displayTag("NewExpression", preVisit);
        else if (cast<NewMemberExpression*>(ast)) display += displayTag("NewMemberExpression", preVisit);
        else if (cast<NotExpression*>(ast)) display += displayTag("NotExpression", preVisit);
        else if (cast<NullExpression*>(ast)) display += displayTag("NullExpression", preVisit);
        else if (cast<NumericLiteral*>(ast)) display += displayTag("NumericLiteral", preVisit);
        else if (cast<NumericLiteralPropertyName*>(ast)) display += displayTag("NumericLiteralPropertyName", preVisit);
        else if (cast<ObjectPattern*>(ast)) display += displayTag("ObjectPattern", preVisit);
        else if (cast<PatternElement*>(ast)) display += displayTag("PatternElement", preVisit);
        else if (cast<PatternElementList*>(ast)) display += displayTag("PatternElementList", preVisit);
        else if (cast<PatternPropertyList*>(ast)) display += displayTag("PatternPropertyList", preVisit);
        else if (cast<PatternProperty*>(ast)) display += displayTag("PatternProperty", preVisit);
        else if (cast<PostDecrementExpression*>(ast)) display += displayTag("PostDecrementExpression", preVisit);
        else if (cast<PostIncrementExpression*>(ast)) display += displayTag("PostIncrementExpression", preVisit);
        else if (cast<PreIncrementExpression*>(ast)) display += displayTag("PreIncrementExpression", preVisit);
        else if (cast<PreDecrementExpression*>(ast)) display += displayTag("PreDecrementExpression", preVisit);
        else if (cast<Program*>(ast)) display += displayTag("Program [WARNING: Not yet taken into account]", preVisit);
        else if (cast<PropertyName*>(ast)) display += displayTag("PropertyName [WARNING: Not yet taken into account]", preVisit);
        else if (cast<RegExpLiteral*>(ast)) display += displayTag("RegExpLiteral", preVisit);
        else if (cast<ReturnStatement*>(ast)) display += displayTag("ReturnStatement", preVisit);
        else if (cast<StatementList*>(ast)) display += displayTag("StatementList", preVisit);
        else if (cast<StringLiteral*>(ast)) display += displayTag("StringLiteral", preVisit);
        else if (cast<StringLiteralPropertyName*>(ast)) display += displayTag("StringLiteralPropertyName", preVisit);
        else if (cast<SwitchStatement*>(ast)) display += displayTag("SwitchStatement", preVisit);
        else if (cast<ThisExpression*>(ast)) display += displayTag("ThisExpression", preVisit);
        else if (cast<ThrowStatement*>(ast)) display += displayTag("ThrowStatement", preVisit);
        else if (cast<TildeExpression*>(ast)) display += displayTag("TildeExpression", preVisit);
        else if (cast<TrueLiteral*>(ast)) display += displayTag("TrueLiteral", preVisit);
        else if (cast<TryStatement*>(ast)) display += displayTag("TryStatement", preVisit);
        else if (cast<TypeOfExpression*>(ast)) display += displayTag("TypeOfExpression", preVisit);
        else if (cast<UiArrayBinding*>(ast)) display += displayTag("UiArrayBinding", preVisit);
        else if (cast<UiArrayMemberList*>(ast)) display += displayTag("UiArrayMemberList", preVisit);
        else if (cast<UiEnumDeclaration*>(ast)) display += displayTag("UiEnumDeclaration", preVisit);
        else if (cast<UiHeaderItemList*>(ast)) display += displayTag("UiHeaderItemList", preVisit);
        else if (cast<UiImport*>(ast)) display += displayTag("UiImport", preVisit);
        //else if (cast<UiEnumMemberList*>(ast)) display += displayTag("UiEnumMemberList [WARNING: Not yet taken into account]", preVisit);
        else if (cast<UiObjectBinding*>(ast)) display += displayTag("UiObjectBinding", preVisit);
        else if (cast<UiObjectDefinition*>(ast)) display += displayTag("UiObjectDefinition", preVisit);
        else if (cast<UiObjectInitializer*>(ast)) display += displayTag("UiObjectInitializer", preVisit);
        else if (cast<UiObjectMemberList*>(ast)) display += displayTag("UiObjectMemberList", preVisit);
        else if (cast<UiParameterList*>(ast)) display += displayTag("UiParameterList", preVisit);
        else if (cast<UiPragma*>(ast)) display += displayTag("UiPragma", preVisit);
        else if (cast<UiProgram*>(ast)) display += displayTag("UiProgram", preVisit);
        else if (cast<UiPublicMember*>(ast)) display += displayTag("UiPublicMember", preVisit);
        else if (cast<UiQualifiedId*>(ast)) display += displayTag("UiQualifiedId", preVisit);
        else if (cast<UiScriptBinding*>(ast)) display += displayTag("UiScriptBinding", preVisit);
        else if (cast<UiSourceElement*>(ast)) display += displayTag("UiSourceElement", preVisit);
        else if (cast<UnaryMinusExpression*>(ast)) display += displayTag("UnaryMinusExpression", preVisit);
        else if (cast<UnaryPlusExpression*>(ast)) display += displayTag("UnaryPlusExpression", preVisit);
        else if (cast<VariableDeclarationList*>(ast)) display += displayTag("VariableDeclarationList", preVisit);
        else if (cast<VariableStatement*>(ast)) display += displayTag("VariableStatement", preVisit);
        else if (cast<VoidExpression*>(ast)) display += displayTag("VoidExpression", preVisit);
        else if (cast<WhileStatement*>(ast)) display += displayTag("WhileStatement", preVisit);
        else if (cast<WithStatement*>(ast)) display += displayTag("WithStatement", preVisit);
        else display += displayTag("[WARNING: Unknown]", preVisit);

        // Semicolons inside these elements? no
        // Semicolons for these elements? no
        if (cast<ArgumentList*>(ast) || cast<Elision*>(ast) || cast<EmptyStatement*>(ast) || cast<FormalParameterList*>(ast) || cast<UiHeaderItemList*>(ast) || cast<UiImport*>(ast) || cast<UiParameterList*>(ast) || cast<UiPragma*>(ast) || cast<UiQualifiedId*>(ast)) {
            preVisit ? addSimpleNode(false) : consumeNode();
        }

        // Semicolons inside these elements? yes
        // Semicolons for these elements? no
        if (cast<FunctionDeclaration*>(ast) || cast<FunctionExpression*>(ast) || cast<UiArrayBinding*>(ast) || cast<UiArrayMemberList*>(ast) || cast<UiObjectBinding*>(ast) || cast<UiObjectDefinition*>(ast) || cast<UiObjectInitializer*>(ast) || cast<UiObjectMemberList*>(ast) || cast<UiProgram*>(ast) || cast<UiSourceElement*>(ast)){
            preVisit ? addSimpleNode(true) : consumeNode();
        }

        // Propagate the possibility of semicolons.
        // It could be deleted but we prefer to treat the complete tree.
        if (cast<Block*>(ast) || cast<DefaultClause*>(ast) || cast<CaseBlock*>(ast) || cast<CaseClauses*>(ast) || cast<Catch*>(ast) || cast<DeleteExpression*>(ast) || cast<Expression*>(ast) || cast<ExpressionStatement*>(ast) || cast<Finally*>(ast) || cast<LabelledStatement*>(ast) || cast<NewExpression*>(ast) || cast<NotExpression*>(ast) || cast<PreDecrementExpression*>(ast) || cast<PreIncrementExpression*>(ast) || cast<StatementList*>(ast) || cast<TildeExpression*>(ast) || cast<TryStatement*>(ast) || cast<TypeOfExpression*>(ast) || cast<UnaryMinusExpression*>(ast) || cast<UnaryPlusExpression*>(ast) || cast<VariableStatement*>(ast) || cast<VoidExpression*>(ast)) {
            preVisit ? addSimpleNode(isPreviousNodeEligible()) : consumeNode();
        }

        if (UiPublicMember* upm = cast<UiPublicMember*>(ast)) {
            preVisit ? addSimpleNode(upm->statement && cast<Block*>(upm->statement)) : consumeNode();
        }

        if (UiScriptBinding* usb = cast<UiScriptBinding*>(ast)) {
            preVisit ? addComplexNode(true, {false, cast<Block*>(usb->statement)}) : consumeNode();
        }

        if (VariableDeclarationList* vdl = cast<VariableDeclarationList*>(ast)) {
            if (preVisit) {
                QVector<bool> canChildrenAddSemicolonsVector;
                for (VariableDeclarationList *it = vdl; it; it = it->next) {
                    canChildrenAddSemicolonsVector.push_back(it->next ? false : isPreviousNodeEligible());
                }
                addComplexNode(isPreviousNodeEligible(), canChildrenAddSemicolonsVector);
            }
            else {
                consumeNode();
            }
        }

        // The three following elements handle themselves the semicolons if there's no expression.
        if (PatternElement* vd = cast<PatternElement*>(ast)) {
            preVisit ? addSimpleNode(isPreviousNodeEligible()) : consumeNode(!vd->initializer);
        }
        if (ReturnStatement* rs = cast<ReturnStatement*>(ast)) {
            preVisit ? addSimpleNode(isPreviousNodeEligible()) : consumeNode(!rs->expression);
        }
        if (ThrowStatement* ts = cast<ThrowStatement*>(ast)) {
            preVisit ? addSimpleNode(isPreviousNodeEligible()) : consumeNode(!ts->expression);
        }

        // All the final elements which are at the end of the tree.
        if (cast<FalseLiteral*>(ast) || cast<IdentifierExpression*>(ast) || cast<IdentifierPropertyName*>(ast) || cast<NullExpression*>(ast) || cast<NumericLiteral*>(ast) || cast<NumericLiteralPropertyName*>(ast) || cast<RegExpLiteral*>(ast) || cast<StringLiteral*>(ast) || cast<StringLiteralPropertyName*>(ast) || cast<ThisExpression*>(ast) || cast<TrueLiteral*>(ast)) {
            preVisit ? addSimpleNode(isPreviousNodeEligible()) : consumeNode(true);
        }

        if (cast<ArrayPattern*>(ast) || cast<BreakStatement*>(ast) || cast<CallExpression*>(ast) || cast<ContinueStatement*>(ast) || cast<FieldMemberExpression*>(ast) || cast<NestedExpression*>(ast) || cast<NewMemberExpression*>(ast) || cast<ObjectPattern*>(ast) || cast<PostIncrementExpression*>(ast) || cast<PostDecrementExpression*>(ast)) {
            preVisit ? addSimpleNode(false) : consumeNode(true);
        }

        if (cast<BinaryExpression*>(ast) || cast<SwitchStatement*>(ast)) {
            preVisit ? addComplexNode(isPreviousNodeEligible(), {false, isPreviousNodeEligible()}) : consumeNode();
        }

        if (CaseClause* cc = cast<CaseClause*>(ast)) {
            preVisit ? (cc->statements ? addComplexNode(isPreviousNodeEligible(), {false, isPreviousNodeEligible()}) : addSimpleNode(false)) : consumeNode();
        }

        if (cast<ArrayMemberExpression*>(ast)) {
            preVisit ? addComplexNode(isPreviousNodeEligible(), {false, false}) : consumeNode(true);
        }

        if (cast<ConditionalExpression*>(ast) || cast<ForEachStatement*>(ast)) {
            preVisit ? addComplexNode(isPreviousNodeEligible(), {false, false, isPreviousNodeEligible()}) : consumeNode();
        }

        if (cast<DoWhileStatement*>(ast)) {
            preVisit ? addComplexNode(isPreviousNodeEligible(), {isPreviousNodeEligible(), false}) : consumeNode();
        }

        if (cast<WhileStatement*>(ast) || cast<WithStatement*>(ast)) {
            preVisit ? addComplexNode(isPreviousNodeEligible(), {false, isPreviousNodeEligible()}) : consumeNode();
        }

        if (ForStatement* fs = cast<ForStatement*>(ast)) {
            if (preVisit) {
                QVector<bool> canChildrenAddSemicolonsVector;
                if (fs->initialiser) {
                    canChildrenAddSemicolonsVector.push_back({false});
                }
                if (fs->declarations) {
                    canChildrenAddSemicolonsVector.push_back({false});
                }
                if (fs->condition) {
                    canChildrenAddSemicolonsVector.push_back({false});
                }
                if (fs->expression) {
                    canChildrenAddSemicolonsVector.push_back({false});
                }
                canChildrenAddSemicolonsVector.push_back({isPreviousNodeEligible()});
                addComplexNode(isPreviousNodeEligible(), canChildrenAddSemicolonsVector);
            }
            else if (!preVisit) {
                consumeNode();
            }
        }

        if (PatternElementList* el = cast<PatternElementList*>(ast)) {
            if (preVisit) {
                QVector<bool> canChildrenAddSemicolonsVector;
                for (PatternElementList *it = el; it; it = it->next) {
                    if (it->elision) {
                        canChildrenAddSemicolonsVector.push_back(false);
                    }
                    canChildrenAddSemicolonsVector.push_back(false);
                }
                addComplexNode(isPreviousNodeEligible(), canChildrenAddSemicolonsVector);
            }
            else {
                consumeNode();
            }
        }

        if (IfStatement* ifs = cast<IfStatement*>(ast)) {
            if (preVisit) {
                QVector<bool> canChildrenAddSemicolonsVector = {false, isPreviousNodeEligible()};
                if (ifs->ko) {
                    canChildrenAddSemicolonsVector.push_back(isPreviousNodeEligible());
                }
                addComplexNode(isPreviousNodeEligible(), canChildrenAddSemicolonsVector);
            }
            else {
                consumeNode();
            }
        }

        display += QString("[") + QString::number(QMLTreeNode.size()) + QString("] ");
        for (int i = 0 ; i < QMLTreeNode.size() ; i++) {
            display += QString("{");
            for (int j = 0 ; j < QMLTreeNode.at(i).size() ; j++) {
                display += QString(QMLTreeNode.at(i).at(j) ? "1" : "0");
            }
            display += QString("}");
            if (QMLTreeNodeType[i] == "node") {
                display += QString("[node]");
            }
        }

        //qDebug() << display;
    }
    /****************************************************************************
     **
     ** Semicolons algorithm - End
     **
     ****************************************************************************/
};

} // anonymous namespace

QString QmlJS::reformat(const Document::Ptr &doc, bool splitLongLines)
{
    Rewriter rewriter(doc, splitLongLines);
    return rewriter(doc->ast());
}

QString QmlJS::reformat(const Document::Ptr &doc, int indentSize, int tabSize, bool splitLongLines)
{
    Rewriter rewriter(doc, splitLongLines);
    rewriter.setIndentSize(indentSize);
    rewriter.setTabSize(tabSize);
    return rewriter(doc->ast());
}
