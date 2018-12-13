include(../core/common.pri)
include(vars.pri)
TARGET   = mbl2bin
TEMPLATE = app
QT      -= gui

QMAKE_RPATHDIR += $ORIGIN/lib
DESTDIR = ../dist

#===================================
HEADERS = util_mbl.h \
#          vertexbuffer.h \
#    humantemplate.h \
#    index.h
    util_common.h

SOURCES = util_mbl_vbuf.cpp \
          util_mbl_morph.cpp \
          util_mbl_measures.cpp \
          util_mbl_expressions.cpp \
          util_mbl_antropometry.cpp \
          util_mbl_main.cpp \
#          vertexbuffer.cpp \
#    humantemplate.cpp \
#    index.cpp
    util_mbl_misc.cpp \
    util_common.cpp

