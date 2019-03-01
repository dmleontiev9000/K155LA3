#pragma once

#include <QtCore/qglobal.h>

#if defined(IDE_LIBRARY)
#  define LIBIDESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBIDESHARED_EXPORT Q_DECL_IMPORT
#endif
