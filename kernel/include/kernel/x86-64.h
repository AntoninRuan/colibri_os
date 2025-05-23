#ifndef X86_64_H
#define X86_64_H

#include <sys/cdefs.h>

#define IRQ_PIT      0x0
#define IRQ_KEYBOARD 0x1

#define IRQ_VECTOR_PIT      0X20
#define IRQ_VECTOR_KEYBOARD 0X21
#define IRQ_HPET_TIMER(n)   (0x30 + n)

#define GDT_ENTRY_KERNEL_CODE 0x8
#define GDT_ENTRY_KERNEL_DATA 0x10
#define GDT_ENTRY_USER_CODE   0x18
#define GDT_ENTRY_USER_DATA   0x20

#define DIVIDE_BY_ZERO         0
#define DEBUG_INTERRUPT        1
#define NON_MASKABLE_INTERRUPT 2
#define BREAKPOINT             3
#define OVERFLOW               4
#define BOUND_RANGE_EXCEEDED   5
#define INVALID_OPCODE         6
#define DEVICE_NOT_AVAILABLE   7
#define DOUBLE_FAULT           8
#define INVALID_TSS            10
#define SEGMENT_NOT_PRESENT    11
#define STAKC_SEGMENT_FAULT    12
#define GENERAL_PROTECTION     13
#define PAGE_FAULT             14
#define X87_FPU_ERROR          16
#define ALIGNMENT_CHECK        17
#define MACHINE_CHECK          18
#define SIMD_ERROR             19

#define APIC_TIMER_INTERRUPT 0x40

#define IA32_EFER 0xC0000080

// Description of flag in Volume 3 Section 5.7 of Intel IA32 Software
// developpers manual
union page_fault_error_code {
    struct {
        u32 present    : 1;
        u32 write      : 1;
        u32 user       : 1;
        u32 rsvd       : 1;
        u32 ifetch     : 1;
        u32 pk         : 1;
        u32 ss         : 1;
        u32 hlat       : 1;
        u32 reserved   : 7;
        u32 sgx        : 1;
        u32 reserved_2 : 16;
    };
    u32 raw;
};
typedef union page_fault_error_code pg_error_t;

struct xmm_reg {
    u64 low;
    u64 high;
};

typedef struct xmm_reg xmm_t;

// Struct use to read the value of register after a pushal
struct registers {
    u64 cr2;
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rsp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;
    xmm_t xmm0;
    xmm_t xmm1;
    xmm_t xmm2;
    xmm_t xmm3;
    xmm_t xmm4;
    xmm_t xmm5;
    xmm_t xmm6;
    xmm_t xmm7;
    xmm_t xmm8;
    xmm_t xmm9;
    xmm_t xmm10;
    xmm_t xmm11;
    xmm_t xmm12;
    xmm_t xmm13;
    xmm_t xmm14;
    xmm_t xmm15;
};

typedef struct registers registers_t;

struct interrupt_frame {
    registers_t registers;

    u64 vector_number;
    u64 error_code;

    u64 iret_rip;
    u64 iret_cs;
    u64 iret_flags;
    u64 iret_rsp;
    u64 iret_ss;
};

typedef struct interrupt_frame int_frame_t;

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
    u64 high, low;
    asm volatile("rdmsr" : "=d"(high), "=a"(low) : "c"(msr_id));
    return (high << 32) | low;
}

static inline void wrmsr(u32 msr_id, u64 msr_value) {
    asm volatile("wrmsr"
                 :
                 : "d"(msr_value >> 32), "a"(msr_value & 0xFFFFFFFF),
                   "c"(msr_id));
}

static inline void __asm_pause() { asm volatile("pause" : : : "memory"); }

static inline void change_pagetable(u64 pagetable_paddr) {
    asm volatile("movq %0, %%cr3" : : "r"(pagetable_paddr));
}

void enable_nx_flag();

#endif  // X86_64_H
