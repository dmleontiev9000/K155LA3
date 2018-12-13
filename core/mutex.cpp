#include "mutex.h"

#include <climits>

#if CONFIG_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/futex.h>

static int futex(int *uaddr, int futex_op, int val,
                 const struct timespec *timeout, int *uaddr2, int val3)
{
   return syscall(SYS_futex, uaddr, futex_op, val,
                  timeout, uaddr2, val3);
}

#endif

#if CONFIG_WIN32
#include "windows.h"
#endif

#if CONFIG_BSD
#include <unistd.h>
#endif

K::Mutex::DataType K::Mutex::tryLock(DataType *ptr,
                                         DataType set,
                                         DataType require,
                                         DataType avoid)
{
    Q_ASSERT((require&avoid) == 0);

    DataType v = __atomic_load_n(ptr, __ATOMIC_CONSUME), nv;
    for(;;) {
        if ((v & (avoid|require)) != require)
            return v;
        nv = v | set;
        if (__atomic_compare_exchange_n(ptr,
                                        &v,
                                        nv,
                                        true,
                                        __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST))
            break;
    }
    return nv;
}
K::Mutex::DataType K::Mutex::lock(DataType *ptr,
                                      DataType set,
                                      DataType require,
                                      DataType avoid,
                                      DataType nowait)
{
    Q_ASSERT((require&avoid) == 0);

    DataType v = __atomic_load_n(ptr, __ATOMIC_CONSUME), nv;
    for(;;) {
        if (v & nowait)
            return v;
        if ((v & (avoid|require)) != require)
        {
            #if CONFIG_LINUX
            futex((int*)&ptr, FUTEX_WAIT, v, NULL, 0, 0);
            #elif CONFIG_BSD
            umtx_sleep((int*)ptr, v, MAX_INT);
            #elif CONFIG_WIN32
            WaitOnAddress(ptr, &v, 4, INFINITE);
            #else
            #error "mutex not implemented for this arch"
            #endif
            v = __atomic_load_n(ptr, __ATOMIC_CONSUME);
            continue;
        }
        nv = v | set;
        if (__atomic_compare_exchange_n(ptr,
                                        &v,
                                        nv,
                                        true,
                                        __ATOMIC_SEQ_CST,
                                        __ATOMIC_SEQ_CST))
            break;
    }
    return nv;
}
K::Mutex::DataType K::Mutex::unlock(DataType *ptr, DataType were_set)
{
    DataType v = __atomic_and_fetch(ptr, ~were_set, __ATOMIC_SEQ_CST);

    #if CONFIG_LINUX
    futex((int*)ptr, FUTEX_WAKE, INT_MAX, 0, 0, 0);
    #elif CONFIG_BSD
    umtx_wakeup((int*)ptr, 0);
    #elif CONFIG_WIN32
    WakeByAddressAll(&ptr);
    #else
    pthread_cond_broadcast(&cond);
    #endif

    return v;
}

