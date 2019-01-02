include(../core/plugin_gui.pri)

TARGET   = gui
TEMPLATE = lib

DEFINES += GUI_LIBRARY

SOURCES += \
    panel.cpp \
    plugin.cpp
HEADERS += \
    conventions.h \
    panel.h \
    plugin.h
