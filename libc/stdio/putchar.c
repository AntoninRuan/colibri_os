#include <stdint.h>
#include <stdio.h>

#ifdef __is_libk
#include <kernel/tty.h>
#endif

int putchar(u8 c) {
#ifdef __is_libk
    terminal_write(c);
#else
// TODO: Implement stdio and write syscall
#endif
    return c;
}
