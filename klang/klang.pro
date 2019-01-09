include(../core/corelib_gui.pri)
include(../boost/boost.pri)
TARGET = klang
TEMPLATE = lib

DEFINES += KLANG_LIBRARY

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments

INCLUDEPATH += $$PWD/../boost
DEPENDPATH  += $$PWD/../boost

INCLUDEPATH += $$PWD/../edit_utils
DEPENDPATH  += $$PWD/../edit_utils
LIBS        += -ledit_utils

SOURCES += \
    lang_node.cpp \
    lang_ref.cpp \
    lang_ctx.cpp \
    syntaxerrors.cpp \
    str.cpp \
    ast.cpp \
    k_ast.cpp \
    tokenizer.cpp \
    k_keywords.cpp \
    k_expr.cpp \
    k_resolve.cpp

HEADERS += \
    klang_global.h \
    lang.h \
    lang_p.h \
    syntaxerrors.h \
    str.h \
    ast.h \
    k_ast.h \
    tokenizer.h \
    k_ast_p.h

win32: INCLUDEPATH += $$_PRO_FILE_PWD_/../3rd_party/boost

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
LIBS += -L$$DESTDIR -lcore
