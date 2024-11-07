#include <kernel/keyboard.h>
#include <kernel/multiboot2.h>

#include "gdt/gdt.h"
#include "interrupt/interrupt.h"
#include "interrupt/apic.h"
#include "interrupt/ioapic.h"

extern uint8_t vector_handler_0x21;

void pre_main(unsigned long magic, unsigned long addr) {
    load_multiboot_info(magic, addr);

    load_idt();                 // Setup interrupts
    enable_lapic();
    read_madt();

    // Enable keyboard support
    init_keyboard();
    set_idt_entry(IRQ_VECTOR_KEYBOARD, (uint32_t) &vector_handler_0x21,
                  GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE)
        );

    set_irq(IRQ_KEYBOARD, IRQ_VECTOR_KEYBOARD, 0, false);
    asm("sti");

    return;
}
