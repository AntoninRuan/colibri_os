#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/x86.h>
#include <stdint.h>

struct interrupt_descriptor {
    uint16_t addr_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t addr_med;
    uint32_t addr_high;
    uint32_t reserved;
} __attribute__((packed));

struct IDTR {
    uint16_t size;
    uint64_t base;
} __attribute__((packed));

struct interrupt_descriptor idt[256] = {0};

void set_idt_entry(uint8_t vector, uint64_t handler_addr, uint16_t selector,
                   uint8_t flags) {
    struct interrupt_descriptor *entry = &idt[vector];

    entry->addr_low = handler_addr & 0xFFFF;
    entry->addr_med = (handler_addr >> 16) & 0xFFFF;
    entry->addr_high = (handler_addr >> 32) & 0xFFFFFFFF;
    entry->flags = flags | 0x80;  // Add the mandatory present flag
    entry->selector = selector;
}

// Defined in isr_wrapper.S
extern uint8_t vector_handler_0;

void load_idt() {
    // Initialized reserved vectors
    for (uint8_t i = 0; i < 32; i++) {
        set_idt_entry(i, (uint64_t)&vector_handler_0 + (i * 16),
                      GDT_ENTRY_KERNEL_CODE,
                      FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    }

    struct IDTR idtr = {0};
    idtr.size = sizeof(struct interrupt_descriptor) * 256 -
                1;  // Size of idt in bytes minus one
    idtr.base = (uint64_t)idt;

    asm volatile("lidt %0" ::"m"(idtr));
    return;
}
