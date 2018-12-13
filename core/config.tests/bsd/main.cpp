//just check for headers for now
#include <sys/kqueue.h>
#include <unistd.h>

//int umtx_sleep(volatile const int *ptr, int value, int timeout);
//int umtx_wakeup(volatile const int *ptr, int count);

int foo(int *bar) {
    return umtx_wakeup(bar, 0) + umtx_sleep(bar, 1, 2);
}
