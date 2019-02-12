TEMPLATE = app

QT += widgets xml

CONFIG += c++14

DEFINES += QT_CREATOR

QT_CREATOR_SIMPLIFIED_SRC = "qt-creator-simplified"

INCLUDEPATH += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs
INCLUDEPATH += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty

HEADERS += src/qmljsreformatter.h \ # src/qmljsreformatter.h overrides the original header
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/cppmodelmanagerbase.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/filesystemwatcher.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/savefile.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/optional.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljscodeformatter.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsmodelmanagerinterface.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsplugindumper.h \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsinterpreter.h

SOURCES += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/AST.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ASTClone.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ASTMatch0.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ASTMatcher.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ASTVisit.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ASTVisitor.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Bind.cpp \  
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Control.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/CoreTypes.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/DiagnosticClient.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/FullySpecifiedType.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Keywords.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Lexer.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Literals.cpp \  
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Matcher.cpp \  
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/MemoryPool.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Name.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Names.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/NameVisitor.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ObjectiveCAtKeywords.cpp \   
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/ObjectiveCTypeQualifiers.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Parser.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/QtContextKeywords.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/SafeMatcher.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Scope.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Symbol.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Symbols.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Templates.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Token.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/TranslationUnit.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/Type.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/3rdparty/cplusplus/TypeVisitor.cpp \ 

SOURCES += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/CppDocument.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/cppmodelmanagerbase.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/CppRewriter.cpp \  
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/DependencyTable.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/DeprecatedGenTemplateInstance.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/FastPreprocessor.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/LookupContext.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/LookupItem.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/Macro.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/NamePrettyPrinter.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/Overview.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/pp-engine.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/PPToken.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/PreprocessorClient.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/PreprocessorEnvironment.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/ResolveExpression.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/TypeOfExpression.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/cplusplus/TypePrettyPrinter.cpp \ 

SOURCES += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/environment.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/fileutils.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/filesystemwatcher.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/hostosinfo.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/json.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/qtcassert.cpp \ 
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/runextensions.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/utils/savefile.cpp

SOURCES += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/languageutils/componentversion.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/languageutils/fakemetaobject.cpp

SOURCES += $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmlerror.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljsast.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljsastvisitor.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmldirparser.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljsengine_p.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljsgrammar.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljslexer.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/parser/qmljsparser.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/persistenttrie.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsbind.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsbundle.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljscodeformatter.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljscontext.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsdialect.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsdocument.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsevaluate.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsfindexportedcpptypes.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsimportdependencies.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsinterpreter.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsmodelmanagerinterface.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsplugindumper.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsqrcparser.cpp \
#   src/qmljsreformatter.cpp overrides this original file
#   $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsreformatter.cpp \
    src/qmljsreformatter.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsscanner.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsscopeastpath.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsscopebuilder.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsscopechain.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljstypedescriptionreader.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsutils.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsvalueowner.cpp \
    $$QT_CREATOR_SIMPLIFIED_SRC/src/libs/qmljs/qmljsviewercontext.cpp

SOURCES += src/main.cpp

TARGET = qmljsreformatter

OTHER_FILES += tests/*.test.qml tests/*.reference.qml tests/*.sh

