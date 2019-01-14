CONFIG   += console
CONFIG   -= app_bundle
TARGET    = test_tokenizer
TEMPLATE  = app
QT       += testlib
QT       -= gui

INCLUDEPATH += $$PWD/../edit_utils
DEPENDPATH  += $$PWD/../edit_utils
LIBS        += -ledit_utils

HEADERS = \
    str.h \
    tokenizer.h \
    test_tokenizer.h

SOURCES = \
    str.cpp \
    tokenizer.cpp \
    test_tokenizer.cpp
