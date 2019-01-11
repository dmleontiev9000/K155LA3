include(../core/corelib_gui.pri)
include(../boost/boost.pri)
TARGET   = lang
TEMPLATE = lib
CONFIG  += staticlib

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
    str.cpp \
    ast.cpp \
    tokenizer.cpp \
    reference.cpp \
    node.cpp \
    context.cpp

HEADERS += \
    klang_global.h \
    str.h \
    ast.h \
    tokenizer.h \
    reference.h \
    reference_p.h \
    node.h \
    node_p.h \
    context.h \
    context_p.h

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
LIBS += -L$$DESTDIR -lcore
