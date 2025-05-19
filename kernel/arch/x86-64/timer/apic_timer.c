#include <cpuid.h>
#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/apic_timer.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/log.h>
#include <kernel/timer.h>
#include <kernel/x86.h>
#include <stdbool.h>
#include <stdint.h>

union lvt_timer_entry {
    struct {
        u8 vector;
        u8                 : 4;
        u8 delivery_status : 1;
        u8                 : 3;
        u8 masked          : 1;
        u8 mode            : 2;
        u16                : 13;
    };
    u32 raw;
};

typedef union lvt_timer_entry lvt_timer_t;

extern void *vector_handler_0x40;
u64 period = 0;

#define CALIBRATION_DURATION 5  // in milliseconds
void calibrate_apic_timer() {
    u32 eax, ebx, ecx, edx;
    if (__get_cpuid(0x15, &eax, &ebx, &ecx, &edx)) {
        period = 1e15 / ecx;
    } else {
        lvt_timer_t entry =
            (lvt_timer_t)read_lapic_register(LAPIC_REG_LVT_TIMER);
        entry.masked = 1;
        entry.mode = 0;
        write_lapic_register(LAPIC_REG_LVT_TIMER, entry.raw);
        write_lapic_register(LAPIC_REG_INITIAL_COUNT, UINT32_MAX);
        millidelay(CALIBRATION_DURATION);
        u64 cycles = read_lapic_register(LAPIC_REG_CURRENT_COUNT);
        cycles = UINT32_MAX - cycles;
        period = (CALIBRATION_DURATION * 1e12) / cycles;
    }
    set_idt_entry(0x40, (u64)&vector_handler_0x40, GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    logf(INFO, "APIC timer period is %d", period);
}

void arm_apic_timer(u64 nano, bool periodic) {
    if (period == 0) calibrate_apic_timer();

    u64 cycle = (nano * 1e6) / period;
    if (cycle >> 32) cycle = UINT32_MAX;
    lvt_timer_t entry = (lvt_timer_t)read_lapic_register(LAPIC_REG_LVT_TIMER);

    entry.mode = (u8)periodic;
    entry.masked = 0;
    entry.vector = 0x40;
    write_lapic_register(LAPIC_REG_LVT_TIMER, entry.raw);

    write_lapic_register(LAPIC_REG_INITIAL_COUNT, cycle);
}
