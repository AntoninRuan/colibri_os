#ifndef KERNEL_H
#define KERNEL_H

#define MAX_CORES 32

#ifndef __is_asm

#include <elf.h>
#include <kernel/process.h>
#include <sys/cdefs.h>

struct kernel_status {
    u32 core_available;
    u32 core_running;

    u32 bsp_id;
    bool nx_flag_enabled;
};

typedef struct kernel_status kernel_status_t;

struct cpu_status {
    u32 id;

    u32 push_off_count;
    u8 int_on;

    proc_t *proc;
};

typedef struct cpu_status cpu_status_t;

extern kernel_status_t kernel_status;
extern cpu_status_t cpu_status[];

#ifdef __arch_x86_64
#include <kernel/arch/x86-64/apic.h>
static inline cpu_status_t *get_cpu() {
    u32 apic_id = get_apic_id();
    if (cpu_status[apic_id].id != apic_id) {
        cpu_status[apic_id].id = apic_id;
    }
    return &cpu_status[get_apic_id()];
}
static inline void disable_interrupt() { asm("cli"); }
static inline void enable_interrupt() { asm("sti"); }
#endif

void push_off();
void pop_off();

void panic(char *);
void idle();
void main(Elf64_Ehdr *);

#endif  // __is_asm
#endif  // KERNEL_H
