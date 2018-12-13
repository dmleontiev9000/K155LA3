#pragma once

#include <QtGlobal>
#include "../core/config.h"

#define _USE_MATH_DEFINES

#if CONFIG_FUNCTIONAL

    #include <functional>
    namespace K {
        using std::function;
        using std::bind;
    }

#elif CONFIG_TR1_FUNCTIONAL

    #include <tr1/functional>
    namespace K {
        using std::tr1::function;
        using std::tr1::bind;
    }

#else

    #error "no usable implementation for std::functional"

#endif

#if CONFIG_WIN32

#define aligned_malloc(s,a) _aligned_malloc((s),(a))
#define aligned_free(p)  _aligned_free(p)

#else

#define aligned_malloc(s,a) aligned_alloc((a),(s))
#define aligned_free(p)   free(p)

#endif


//#if QT_VERSION <  0x050900
//#error "Qt version must be at least 5.9.1"
//#endif

#if defined(Q_OS_WIN)
#define LIB_PREFIX ""
#define LIB_SUFFIX ".dll"
#elif defined(Q_OS_MAC)
#define LIB_PREFIX "lib"
#define LIB_SUFFIX ".dylib"
#else
#define LIB_PREFIX "lib"
#define LIB_SUFFIX ".so"
#endif

#define SLOT_TAG(xxxx) (#xxxx)
