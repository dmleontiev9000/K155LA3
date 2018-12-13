#ifndef WIDGETS_GLOBAL_H
#define WIDGETS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(COREGUI_LIBRARY)
#  define COREGUISHARED_EXPORT Q_DECL_EXPORT
#else
#  define COREGUISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // WIDGETS_GLOBAL_H
