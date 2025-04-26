#ifndef X86_64_H
#define X86_64_H

#include <stdint.h>

#define IRQ_PIT      0x0
#define IRQ_KEYBOARD 0x1
#define IRQ_HPET_TIMER(n) (0x30 + n)

#define IRQ_VECTOR_PIT      0X20
#define IRQ_VECTOR_KEYBOARD 0X21

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

#define IA32_EFER              0xC0000080

// Description of flag in Volume 3 Section 5.7 of Intel IA32 Software developpers manual
union page_fault_error_code {
    struct {
        uint32_t present    :1;
        uint32_t write      :1;
        uint32_t user       :1;
        uint32_t rsvd       :1;
        uint32_t ifetch     :1;
        uint32_t pk         :1;
        uint32_t ss         :1;
        uint32_t hlat       :1;
        uint32_t reserved   :7;
        uint32_t sgx        :1;
        uint32_t reserved_2 :16;
    };
    uint32_t raw;
};
typedef union page_fault_error_code pg_error_t;

// Struct use to read the value of register after a pushal
struct registers_t {
    uint64_t cr2;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;
};

struct interrupt_frame {
    struct registers_t registers;

    uint64_t vector_number;
    uint64_t error_code;

    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
};

struct cpu_status {
    uint32_t bsp_id;
    uint32_t core_available;

    uint32_t core_running;
    bool nx_flag_enabled;
};
typedef struct cpu_status cpu_status_t;

extern cpu_status_t cpu_status;

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile ("inb %w1, %b0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void iowait() {
    outb(0x80, 0);
}

static inline uint64_t rdmsr(uint32_t msr_id) {
    uint64_t high, low;
    asm volatile ("rdmsr" : "=d" (high), "=a" (low) : "c"(msr_id));
    return (high << 32) | low;
}

static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
    asm volatile ("wrmsr" : : "d" (msr_value >> 32), "a" (msr_value & 0xFFFFFFFF), "c"(msr_id));
}

static inline void __asm_pause() {
    asm volatile ("pause" : : : "memory");
}

void enable_nx_flag();

#endif // X86_64_H
