#include <kernel/acpi.h>
#include <kernel/arch/x86-64/hpet.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/arch/x86-64/ioapic.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/timer.h>
#include <stddef.h>
#include <stdint.h>

extern u8 vector_handler_0x30;
extern u8 vector_handler_0x31;
extern u8 vector_handler_0x32;
extern u8 vector_handler_0x33;
extern u8 vector_handler_0x34;
extern u8 vector_handler_0x35;
extern u8 vector_handler_0x36;
extern u8 vector_handler_0x37;
extern u8 vector_handler_0x38;
extern u8 vector_handler_0x39;
extern u8 vector_handler_0x3A;
extern u8 vector_handler_0x3B;
extern u8 vector_handler_0x3C;
extern u8 vector_handler_0x3D;
extern u8 vector_handler_0x3E;
u64 hpet_vec_handler[32] = {
    (u64)&vector_handler_0x30, (u64)&vector_handler_0x31,
    (u64)&vector_handler_0x32, (u64)&vector_handler_0x33,
    (u64)&vector_handler_0x34, (u64)&vector_handler_0x35,
    (u64)&vector_handler_0x36, (u64)&vector_handler_0x37,
    (u64)&vector_handler_0x38, (u64)&vector_handler_0x39,
    (u64)&vector_handler_0x3A, (u64)&vector_handler_0x3B,
    (u64)&vector_handler_0x3C, (u64)&vector_handler_0x3D,
    (u64)&vector_handler_0x3E};

volatile u64 *hpet_base_addr;
u32 hpet_period;
u8 timer_count;
bool hpet_available = false;

u64 read_hpet_register(u16 reg) { return *(hpet_base_addr + reg); }

void write_hpet_register(u16 reg, u64 value) {
    *(hpet_base_addr + reg) = value;
}

void enable_hpet() {
    u64 config = read_hpet_register(GENERAL_CONFIGURATION_REG);
    config |= 1;
    write_hpet_register(GENERAL_CONFIGURATION_REG, config);
}

void configure_timer(u8 timer, timer_config_t conf) {
    // Timer is not available
    if (timer >= timer_count) return;

    write_hpet_register(TIMER_CONFIG_REG(timer), conf.raw);
}

u64 poll_hpet() { return read_hpet_register(MAIN_COUNTER_VALUE_REG); }

void sleep_polled_hpet(u64 femto) {
    u64 cycle = femto / hpet_period;
    ;
    u64 main_counter = poll_hpet();
    u64 target = main_counter + cycle;

    if (main_counter > target)
        while (poll_hpet() > target);

    while (poll_hpet() < target);
    return;
}

int arm_hpet_timer(u8 timer, u64 femto, bool periodic) {
    push_off();
    if (timer >= timer_count) return 1;
    u64 cycle = femto / (u64)hpet_period;
    timer_config_t config =
        (timer_config_t)read_hpet_register(TIMER_CONFIG_REG(timer));

    if (config.int_route_cnf == 0) {
        u8 irq_used = 9;
        u32 available_irq = config.int_route_cap;
        available_irq >>= 9;
        while (available_irq) {
            while ((available_irq & 1) == 0) {
                irq_used++;
                available_irq >>= 1;
            }
            if (!set_irq(irq_used, 0x30 + timer, DEST_LOGICAL, 0xFF, false))
                break;
        }
        if (available_irq == 0) return 1;
        config.int_route_cnf = irq_used;
    }

    config.int_enb_cnf = true;
    if (periodic) {
        if (!config.per_int_cap) return 1;
        config.per_int_cnf = true;
        config.val_set_cnf = true;
    } else {
        config.per_int_cnf = false;
        config.val_set_cnf = false;
    }

    write_hpet_register(TIMER_COMPARATOR_REG(timer), poll_hpet() + cycle);
    write_hpet_register(TIMER_CONFIG_REG(timer), config.raw);
    write_hpet_register(TIMER_COMPARATOR_REG(timer), poll_hpet() + cycle);
    if (periodic) write_hpet_register(TIMER_COMPARATOR_REG(timer), cycle);

    pop_off();
    return 0;
}

int setup_hpet() {
    acpi_sdt_header_t *header = find_table("HPET");
    if (!validate_sdt(header)) return 1;

    hpet_acpi_t *hpet_table = (hpet_acpi_t *)header;

    hpet_base_addr =
        map_mmio(&kernel_vmm, hpet_table->base_addr.addr, 0x400, true);
    if (hpet_base_addr == NULL) {
        logf(ERROR, "MMIO mapping for hpet failed");
        return 1;
    }

    u64 general_cap = read_hpet_register(GENERAL_CAPABILTIES_REG);
    timer_count = ((general_cap >> 8) & 0x1F) + 1;
    hpet_period = (u32)(general_cap >> 32);
    logf(INFO, "HPET found with %d timer available", timer_count);

    enable_hpet();

    for (u8 i = 0; i < timer_count; i++) {
        set_idt_entry(HPET_BASE_INT_VEC + i, hpet_vec_handler[i],
                      GDT_ENTRY_KERNEL_CODE,
                      FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));
    }

    return 0;
}
