CONFIG   += console
CONFIG   -= app_bundle
TARGET    = test_string
TEMPLATE  = app
QT       += testlib
QT       -= gui

INCLUDEPATH += $$PWD/../editutils
DEPENDPATH  += $$PWD/../editutils
LIBS        += -leditutils

HEADERS = \
    str.h \
    test_string.h

SOURCES = \
    str.cpp \
    test_string.cpp

QMAKE_RPATHDIR += $ORIGIN/lib
DESTDIR = ../dist
win32: LIBS += -L../dist
else : LIBS += -L../dist/lib
