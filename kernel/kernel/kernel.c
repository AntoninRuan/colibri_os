#include <stdint.h>
#include <stdio.h>

#include <kernel/multiboot2.h>

void main(unsigned long magic, unsigned long addr) {
    load_multiboot_info(magic, addr);

    printf("Hello world!");
}
