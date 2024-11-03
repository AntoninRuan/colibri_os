#include "gdt/gdt.h"
#include "interrupt/interrupt.h"

void pre_main() {
    load_gdt();                 // Load gdt from gdt.c
    load_idt();                 // Setup interrupts

    return;
}
