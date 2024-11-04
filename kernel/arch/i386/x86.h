#ifndef X86_H
#define X86_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %w1 %b0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void iowait() {
    outb(0x80, 0);
}

static inline uint64_t rdmsr(uint32_t msr_id) {
    uint64_t msr_value;
    asm volatile ("rdmsr" : "=A" (msr_value) : "c"(msr_id));
    return msr_value;
}

static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
    asm volatile ("wrmsr" : : "A" (msr_value), "c"(msr_id));
}

#endif // X86_H
