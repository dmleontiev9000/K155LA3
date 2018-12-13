include(../core/corelib.pri)
include(../core/common_gui.pri)

TARGET     = coregui
TEMPLATE   = lib

DEFINES += COREGUI_LIBRARY

SOURCES += \
    style.cpp \
    switchbar.cpp \
    fancymenu.cpp \
    action.cpp \
    digitalclock.cpp \
    subtreemodel.cpp \
    detaileditemdelegate.cpp \
    scrollwidget.cpp \
    clock.cpp \
    checklist.cpp \
    icons.cpp

HEADERS +=\
    fancymenu.h \
    style.h \
    switchbar.h \
    action.h \
    digitalclock.h \
    subtreemodel.h \
    detaileditemdelegate.h \
    scrollwidget.h \
    clock.h \
    coregui_global.h \
    checklist.h \
    icons.h
