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
        uint8_t vector;
        uint8_t                 : 4;
        uint8_t delivery_status : 1;
        uint8_t                 : 3;
        uint8_t masked          : 1;
        uint8_t mode            : 2;
        uint16_t                : 13;
    };
    uint32_t raw;
};

typedef union lvt_timer_entry lvt_timer_t;

extern void *vector_handler_0x40;
uint64_t period = 0;

#define CALIBRATION_DURATION 5  // in milliseconds
void calibrate_apic_timer() {
    uint32_t eax, ebx, ecx, edx;
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
        uint64_t cycles = read_lapic_register(LAPIC_REG_CURRENT_COUNT);
        cycles = UINT32_MAX - cycles;
        period = (CALIBRATION_DURATION * 1e12) / cycles;
    }
    set_idt_entry(0x40, (uint64_t)&vector_handler_0x40, GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    logf(INFO, "APIC timer period is %d", period);
}

void arm_apic_timer(uint64_t nano, bool periodic) {
    if (period == 0) calibrate_apic_timer();

    uint64_t cycle = (nano * 1e6) / period;
    if (cycle >> 32) cycle = UINT32_MAX;
    lvt_timer_t entry = (lvt_timer_t)read_lapic_register(LAPIC_REG_LVT_TIMER);

    entry.mode = (uint8_t)periodic;
    entry.masked = 0;
    entry.vector = 0x40;
    write_lapic_register(LAPIC_REG_LVT_TIMER, entry.raw);

    write_lapic_register(LAPIC_REG_INITIAL_COUNT, cycle);
}
