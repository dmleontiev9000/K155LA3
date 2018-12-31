include(../core/corelib_gui.pri)

TARGET = klang
TEMPLATE = lib

DEFINES += KLANG_LIBRARY

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments

INCLUDEPATH += $$PWD/../edit_utils
DEPENDPATH  += $$PWD/../edit_utils
LIBS        += -ledit_utils

SOURCES += \
    lang_node.cpp \
    lang_ref.cpp \
    lang_ctx.cpp \
    syntaxerrors.cpp \
    str.cpp \
    eval.cpp \
    ast.cpp \
    k_tokenizer.cpp \
    k_ast.cpp

HEADERS += \
    klang_global.h \
    lang.h \
    lang_p.h \
    syntaxerrors.h \
    str.h \
    eval.h \
    ast.h \
    k_tokenizer.h \
    k_ast.h

win32: INCLUDEPATH += $$_PRO_FILE_PWD_/../3rd_party/boost

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
LIBS += -L$$DESTDIR -lcore
