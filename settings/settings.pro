include(../core/plugin_gui.pri)

TARGET   = settings
TEMPLATE = lib

DEFINES += IDE_LIBRARY

SOURCES += \
    plugin.cpp

HEADERS +=\
    conventions.h \
    plugin.h

