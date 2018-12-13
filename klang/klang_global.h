#pragma once

#include <QtCore/qglobal.h>

#if defined(KLANG_LIBRARY)
#  define K_LANG_EXPORT Q_DECL_EXPORT
#else
#  define K_LANG_EXPORT Q_DECL_IMPORT
#endif
