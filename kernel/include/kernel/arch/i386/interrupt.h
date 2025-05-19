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

void set_idt_entry(u8 vector, u32 handler_addr, u16 selector, u8 flags);
void load_idt();

#endif  // INTERRUPT_H
