include(../core/corelib_gui.pri)
include(../boost/boost.pri)
TARGET   = klang
TEMPLATE = lib
CONFIG  += staticlib
DEFINES += KLANG_LIBRARY

INCLUDEPATH += $$PWD/../documents
DEPENDPATH  += $$PWD/../documents
LIBS        += -ldocuments

INCLUDEPATH += $$PWD/../boost
DEPENDPATH  += $$PWD/../boost

INCLUDEPATH += $$PWD/../edit_utils
DEPENDPATH  += $$PWD/../edit_utils
LIBS        += -ledit_utils

SOURCES += \
    k_ast.cpp \
    k_keywords.cpp \
    k_expr.cpp \
    k_resolve.cpp

HEADERS += \
    klang_global.h \
    k_ast.h \
    k_ast_p.h

INCLUDEPATH += $$PWD/../core
DEPENDPATH += $$PWD/../core
LIBS += -L$$DESTDIR -lcore
