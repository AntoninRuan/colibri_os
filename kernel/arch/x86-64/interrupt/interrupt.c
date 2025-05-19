#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/x86.h>
#include <stdint.h>

struct interrupt_descriptor {
    u16 addr_low;
    u16 selector;
    u8 ist;
    u8 flags;
    u16 addr_med;
    u32 addr_high;
    u32 reserved;
} __attribute__((packed));

struct IDTR {
    u16 size;
    u64 base;
} __attribute__((packed));

struct interrupt_descriptor idt[256] = {0};

void set_idt_entry(u8 vector, u64 handler_addr, u16 selector, u8 flags) {
    struct interrupt_descriptor *entry = &idt[vector];

    entry->addr_low = handler_addr & 0xFFFF;
    entry->addr_med = (handler_addr >> 16) & 0xFFFF;
    entry->addr_high = (handler_addr >> 32) & 0xFFFFFFFF;
    entry->flags = flags | 0x80;  // Add the mandatory present flag
    entry->selector = selector;
}

// Defined in isr_wrapper.S
extern u8 vector_handler_0;

void load_idt() {
    // Initialized reserved vectors
    for (u8 i = 0; i < 32; i++) {
        set_idt_entry(i, (u64)&vector_handler_0 + (i * 16),
                      GDT_ENTRY_KERNEL_CODE,
                      FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    }

    struct IDTR idtr = {0};
    idtr.size = sizeof(struct interrupt_descriptor) * 256 -
                1;  // Size of idt in bytes minus one
    idtr.base = (u64)idt;

    asm volatile("lidt %0" ::"m"(idtr));
    return;
}
