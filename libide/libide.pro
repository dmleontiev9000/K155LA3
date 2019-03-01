include(../core/corelib_gui.pri)

TARGET     = ide
TEMPLATE   = lib

DEFINES   += IDE_LIBRARY
QT        += sql

SOURCES += \
    document.cpp \
    openedmodel.cpp \
    bookmarkmodel.cpp \
    editor.cpp \
    editordocumentmodel.cpp \
    assetpool.cpp

HEADERS +=\
    document.h \
    openedmodel.h \
    bookmarkmodel.h \
    editor.h \
    editordocumentmodel.h \
    assetpool.h \
    libide_global.h
