#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

// Privilege field, CPU privilege level required to access the interrupt via
// the INT instruction (ignored by hardware interrupt)
#define FLAGS_DPL(x) (x << 5)

// Define the type of gate the interrupt descriptor describe
// Five valids value:
//     0x5: Task Gate
//     0x6: 16-bit Interrupt
//     0x7: 16-bit Trap
//     0xE: 32-bit Interrupt
//     0xF: 32-bit Trap
// In a interrupt other interrupts are disable whereas in a trap interrupt
// can still occurs
#define FLAGS_GATE_TYPE(x) (x)

struct interrupt_descriptor {
    uint16_t addr_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t addr_high;
} __attribute__((packed));

struct IDTR {
    uint16_t size;
    uint32_t base;
} __attribute__((packed));

#include "../cpu.h"

struct interrupt_frame {
    struct registers_t registers;

    uint32_t vector_number;
    uint32_t error_code;
};

void set_idt_entry(uint8_t vector, uint32_t  handler_addr, uint16_t selector, uint8_t flags);
void load_idt();

#endif // INTERRUPT_H
