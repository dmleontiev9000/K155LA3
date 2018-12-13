include(../core/common.pri)
win32:DESTDIR   = ../dist
else: DESTDIR   = ../dist/lib
win32:LIBS     += -L$$DESTDIR -lcore
else: LIBS     += -L$$DESTDIR/../lib -lcore
QMAKE_RPATHDIR += $ORIGIN
INCLUDEPATH    += $$PWD/../core
DEPENDPATH     += $$PWD/../core
