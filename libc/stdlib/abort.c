#include <stdlib.h>

#ifdef __is_libk
#include <kernel/kernel.h>
#endif // __is_libk

__attribute__((__noreturn__))
void abort(void) {
    #ifdef __is_libk
    panic("kernel: panic: abort()\n");
    # else
    // TODO: Abnormally terminate the process as if by SIGABRT
    printf("abort()\n");
    #endif
    while(1) { }
    __builtin_unreachable();
}
