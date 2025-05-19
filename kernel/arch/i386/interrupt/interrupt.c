#include <kernel/arch/i386/interrupt.h>
#include <kernel/x86.h>
#include <stdint.h>

struct interrupt_descriptor {
    u16 addr_low;
    u16 selector;
    u8 reserved;
    u8 flags;
    u16 addr_high;
} __attribute__((packed));

struct IDTR {
    u16 size;
    u32 base;
} __attribute__((packed));

struct interrupt_descriptor idt[256] = {0};

void set_idt_entry(u8 vector, u32 handler_addr, u16 selector, u8 flags) {
    struct interrupt_descriptor *entry = &idt[vector];

    entry->addr_low = handler_addr & 0xFFFF;
    entry->addr_high = (handler_addr >> 16) & 0xFFFF;
    entry->flags = flags | 0x80;  // Add the mandatory present flag
    entry->selector = selector;
}

// Defined in isr_wrapper.S
extern u8 vector_handler_0;

void load_idt() {
    // Initialized reserved vectors
    for (u8 i = 0; i < 32; i++) {
        set_idt_entry(i, (u32)&vector_handler_0 + (i * 16),
                      GDT_ENTRY_KERNEL_CODE,
                      FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    }

    struct IDTR idtr = {0};
    idtr.size = 0x7FF;  // Size fo idt in bytes minus one
    idtr.base = (u32)idt;

    asm volatile("lidt %0" ::"m"(idtr));
    return;
}
