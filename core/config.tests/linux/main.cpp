#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <linux/futex.h>

static int
futex(int *uaddr, int futex_op, int val,
     const struct timespec *timeout, int *uaddr2, int val3)
{
   return syscall(SYS_futex, uaddr, futex_op, val,
                  timeout, uaddr, val3);
}

int foo(int *bar) {
    return futex(bar, FUTEX_WAIT, 0, 0, 0, 0);
}
