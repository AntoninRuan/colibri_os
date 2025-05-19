#include <kernel/arch/i386/apic.h>
#include <kernel/arch/i386/interrupt.h>
#include <kernel/arch/i386/ioapic.h>
#include <kernel/keyboard.h>
#include <kernel/multiboot2.h>
#include <kernel/x86.h>

extern u8 vector_handler_0x21;

void pre_main(unsigned long magic, unsigned long addr) {
    load_multiboot_info(magic, addr);

    load_idt();  // Setup interrupts
    enable_lapic();
    read_madt();

    // Enable keyboard support
    init_keyboard();
    set_idt_entry(IRQ_VECTOR_KEYBOARD, (u32)&vector_handler_0x21,
                  GDT_ENTRY_KERNEL_CODE, FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    set_irq(IRQ_KEYBOARD, IRQ_VECTOR_KEYBOARD, 0, false);
    asm("sti");

    return;
}
