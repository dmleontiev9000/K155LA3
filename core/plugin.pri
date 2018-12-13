include(../core/common.pri)
DESTDIR         = ../dist/plugins
QMAKE_RPATHDIR += $ORIGIN/../lib
win32:LIBS     += -L$$DESTDIR/.. -lcore
else: LIBS     += -L$$DESTDIR/../lib -lcore
INCLUDEPATH    += $$PWD/../core
DEPENDPATH     += $$PWD/../core

