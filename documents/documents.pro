include(../core/corelib_gui.pri)

TARGET     = documents
TEMPLATE   = lib
DEFINES   += DOCUMENTS_LIBRARY
QT        += sql

SOURCES += \
    document.cpp \
    openedmodel.cpp \
    bookmarkmodel.cpp \
    editor.cpp \
    editordocumentmodel.cpp

HEADERS +=\
    documents_global.h \
    document.h \
    openedmodel.h \
    bookmarkmodel.h \
    editor.h \
    editordocumentmodel.h
