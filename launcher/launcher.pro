include(../core/common.pri)
include(../core/common_gui.pri)

TARGET = launcher
TEMPLATE = app

SOURCES += main.cpp \
    core.cpp \
    pluginhandle.cpp \
    worker.cpp

HEADERS += \
    core.h \
    pluginhandle.h \
    worker.h

QMAKE_RPATHDIR += $ORIGIN/lib
DESTDIR = ../dist

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core

win32: LIBS += -L../dist
else : LIBS += -L../dist/lib
LIBS += -lcore -lcoregui

mkshare.target       = ".mkshare"
mkshare.commands     = $$makedir($$OUT_PWD/../dist/share) && $$tag(.mkshare)
cpshare.target       = ".cpshare"
cpshare.commands     = $$QMAKE_COPY_DIR $$shell_path($$PWD/../share) $$shell_path($$OUT_PWD/../dist/share) && $$tag(.cpshare)
cpshare.depends      = mkshare
QMAKE_EXTRA_TARGETS += mkshare cpshare
PRE_TARGETDEPS      = $$mkshare.target $$cpshare.target
QMAKE_CLEAN         += $$mkshare.target $$cpshare.target

win32 {
CONFIG += console
QMAKE_POST_LINK = windeployqt --force --qthelp --sql --xml --network --opengl --dir ../dist/ --libdir ../dist --plugindir ../dist/plugins --compiler-runtime ../dist/launcher.exe
}
