include(../core/plugin_gui.pri)

TARGET   = ide
TEMPLATE = lib

DEFINES += IDE_LIBRARY

SOURCES += \
    projectmodel.cpp \
    plugin.cpp

HEADERS +=\
    conventions.h \
    projectmodel.h \
    plugin.h

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments
