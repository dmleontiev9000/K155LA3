include(../core/corelib_gui.pri)
include(../boost/boost.pri)
TARGET   = lang
TEMPLATE = lib
CONFIG  += staticlib

DEFINES += KLANG_LIBRARY

INCLUDEPATH += $$PWD/../boost
DEPENDPATH  += $$PWD/../boost

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments

INCLUDEPATH += $$PWD/../editutils
DEPENDPATH  += $$PWD/../editutils
LIBS        += -leditutils

SOURCES += \
    str.cpp \
    ast.cpp \
    tokenizer.cpp \
    reference.cpp \
    node.cpp \
    context.cpp \
    context_fin.cpp \
    context_inv.cpp \
    context_queue.cpp \
    file.cpp \
    context_proc.cpp \
    strmap.cpp \
    context_nm.cpp

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
    context_p.h \
    file.h \
    file_p.h \
    strmap.h

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
LIBS += -L$$DESTDIR -lcore
