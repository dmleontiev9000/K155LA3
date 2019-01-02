include(../core/plugin_gui.pri)

QT      += help
TARGET   = help
TEMPLATE = lib

DEFINES += HELP_LIBRARY

SOURCES += \
    plugin.cpp

HEADERS +=\
    conventions.h \
    plugin.h
