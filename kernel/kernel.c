#include <kernel/kernel.h>
#include <kernel/log.h>
#include <stdint.h>

__attribute__((__noreturn__)) void panic(char *msg) {
    log(ERROR, msg);
    while (1);
    __builtin_unreachable();
}

void main(void) { while (1); }
