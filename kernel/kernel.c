#include <stdint.h>
#include <stdio.h>

__attribute__((__noreturn__))
void panic(char *msg) {
    printf(msg);
    while(1);
    __builtin_unreachable();
}

void main(void) {
    // printf("Hello world!");

    while(1);
}
