#pragma once

#include <QtCore/qglobal.h>

#if defined(K_CORE_LIBRARY)
#  define K_CORE_EXPORT Q_DECL_EXPORT
#else
#  define K_CORE_EXPORT Q_DECL_IMPORT
#endif

