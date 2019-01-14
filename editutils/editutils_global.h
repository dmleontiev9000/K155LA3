#pragma once

#include <QtCore/qglobal.h>

#if defined(EDITUTILS_LIBRARY)
#  define EDITUTILSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EDITUTILSSHARED_EXPORT Q_DECL_IMPORT
#endif
