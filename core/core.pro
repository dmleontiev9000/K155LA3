#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T16:25:07
#
#-------------------------------------------------
include(common.pri)

TARGET     = core
TEMPLATE   = lib
win32:DESTDIR   = ../dist
else: DESTDIR   = ../dist/lib

CONFIG_64_ = 0
if (syntaxTest(64)) {
    CONFIG_64_ = 1
}
CONFIG_FUNCTIONAL_ = 0
CONFIG_TR1_FUNCTIONAL_ = 0
if (syntaxTest(functional)) {
    CONFIG_FUNCTIONAL_ = 1
} else : if (syntaxTest(tr1_functional)) {
    CONFIG_TR1_FUNCTIONAL_ = 1
}
CONFIG_WIN32_ = 0
CONFIG_MSVC_  = 0
CONFIG_BSD_   = 0
CONFIG_LINUX_ = 0

win32-msvc* {
    CONFIG_WIN32_ = 1
    CONFIG_MSVC_  = 1
} else:win32 {
    CONFIG_WIN32_ = 1
#    LIBS += -l$$PWD/synhcronization.lib
} else {
    if (syntaxTest(bsd)) {
        CONFIG_BSD_ = 1
    } else : if (syntaxTest(linux)) {
        CONFIG_LINUX_ = 1
    } else : error("Cannot detect OS")
}
HEADERS += \

config_h.input  = config.h.in
config_h.output = config.h
QMAKE_SUBSTITUTES += config_h
K_VERSION_ = "1.0"

OTHER_FILES += \
    common.pri \
    common_gui.pri \
    plugin.pri \
    plugin_gui.pri \
    corelib.pri \
    corelib_gui.pri \
    functional/main.cpp \
    tr1_functional/main.cpp \
    linux/main.cpp \
    bsd/main.cpp \
    64/main.cpp

DEFINES += K_CORE_LIBRARY

SOURCES += \
    ifcore.cpp \
    ifplugin.cpp \
    ifautoplugin.cpp \
    utils.cpp

HEADERS +=\
    config.h.in \
    compat.h \
    core_global.h \
    interfaces.h \
    utils.h

#SOURCES += mutex.cpp
#HEADERS += mutex.h

DISTFILES += \
    windows_unzip.vbs \
    windows_wget.vbs \
    windows_mkdir.vbs

