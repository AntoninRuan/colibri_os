#ifndef KERNEL_H
#define KERNEL_H

#define MAX_CORES 32

#ifndef __is_asm

#include <stdint.h>
#ifdef __arch_x86_64
#include <kernel/arch/x86-64/apic.h>
static inline uint32_t get_cpu_id() { return get_apic_id(); }
static inline void disable_interrupt() { asm("cli"); }
static inline void enable_interrupt() { asm("sti"); }
#endif

void panic(char *);
void main();

#endif  // __is_asm
#endif  // KERNEL_H
