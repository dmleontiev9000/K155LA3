include(../core/plugin.pri)
include(../core/common_gui.pri)

QMAKE_EXTRA_TARGETS        += download_makehuman
download_makehuman.target   = .download_makehuman
download_makehuman.commands =
PRE_TARGETDEPS             += .download_makehuman

QMAKE_EXTRA_TARGETS += build_makehuman
build_makehuman.target      = .build_makehuman
build_makehuman.commands    =
build_makehuman.depends     =

INCLUDEPATH += $$PWD/../coregui
DEPENDPATH  += $$PWD/../coregui
LIBS        += -lcoregui
