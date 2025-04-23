#include <stdint.h>

#include <kernel/memory/vmm.h>
#include <kernel/log.h>

__attribute__((__noreturn__))
void panic(char *msg) {
    log(ERROR, msg);
    while(1);
    __builtin_unreachable();
}

void main(void) {

    while(1);
}
