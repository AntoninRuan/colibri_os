#ifndef X86_H
#define X86_H

#include <sys/cdefs.h>

#define IRQ_PIT      0x0
#define IRQ_KEYBOARD 0x1

#define IRQ_VECTOR_PIT      0X20
#define IRQ_VECTOR_KEYBOARD 0X21

#define GDT_ENTRY_KERNEL_CODE 0x8
#define GDT_ENTRY_KERNEL_DATA 0x10
#define GDT_ENTRY_USER_CODE   0x18
#define GDT_ENTRY_USER_DATA   0x20

// Struct use to read the value of general purposer register after a pushal
struct registers_t {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecpx;
    u32 eax;
};

struct interrupt_frame {
    struct registers_t registers;

    u32 vector_number;
    u32 error_code;
};

static inline void outb(u16 port, u8 value) {
    asm volatile("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory");
}

static inline u8 inb(u16 port) {
    u8 value;
    asm volatile("inb %w1, %b0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void iowait() { outb(0x80, 0); }

static inline u64 rdmsr(u32 msr_id) {
    u64 msr_value;
    asm volatile("rdmsr" : "=A"(msr_value) : "c"(msr_id));
    return msr_value;
}

static inline void wrmsr(u32 msr_id, u64 msr_value) {
    asm volatile("wrmsr" : : "A"(msr_value), "c"(msr_id));
}

#endif  // X86_H
