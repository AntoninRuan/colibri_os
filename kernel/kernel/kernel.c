#include <stdio.h>

#include <kernel/tty.h>

void main(void) {
    terminal_initialize();
    printf("Hello kernel world!\n");
}
