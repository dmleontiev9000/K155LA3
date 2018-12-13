include(../core/plugin_gui.pri)
include(vars.pri)
TARGET   = ide
TEMPLATE = lib

DEFINES += MAKEGAME_LIBRARY

#
# this plugin requires manuel bastion labs files to generate mob models
#
compile_mbl.target = .mbl_compile
compile_mbl.commands = $$shell_path($$OUT_PWD/../dist/mbl2bin) $$OUT_PWD/manuelbastionilab/data $$MBLABS_DIR && $$tag($$compile_mbl.target)
QMAKE_EXTRA_TARGETS  += compile_mbl
PRE_TARGETDEPS       += $$compile_mbl.target
#===================================

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments

SOURCES += \
    plugin.cpp \
    projectdialog.cpp \
    projectconfiguration.cpp \
    morphingengine.cpp \
    hdata.cpp \
    humanoid.cpp \
    util.cpp \
    h_character.cpp \
    morphs.cpp \
    texture.cpp \
    vertexbuffer.cpp \
    renderplan.cpp \
    humanoidfactory.cpp \
    util_common.cpp

HEADERS += \
    plugin.h \
    projectdialog.h \
    projectconfiguration.h \
    configuration.h \
    morphingengine.h \
    hdata.h \
    humanoid.h \
    util.h \
    h_character.h \
    morphs.h \
    texture.h \
    vertexbuffer.h \
    renderplan.h \
    humanoidfactory.h \
    util_common.h

