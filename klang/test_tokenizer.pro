CONFIG   += console
CONFIG   -= app_bundle
TARGET    = test_tokenizer
TEMPLATE  = app
QT       += testlib
QT       -= gui

INCLUDEPATH += $$PWD/../editutils
DEPENDPATH  += $$PWD/../editutils
LIBS        += -leditutils

HEADERS = \
    str.h \
    tokenizer.h \
    test_tokenizer.h

SOURCES = \
    str.cpp \
    tokenizer.cpp \
    test_tokenizer.cpp

QMAKE_RPATHDIR += $ORIGIN/lib
DESTDIR = ../dist
win32: LIBS += -L../dist
else : LIBS += -L../dist/lib

