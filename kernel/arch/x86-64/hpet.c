#include <kernel/x86.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/acpi.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/timer.h>

#include <kernel/arch/x86-64/hpet.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/arch/x86-64/ioapic.h>

extern uint8_t vector_handler_0x30;
extern uint8_t vector_handler_0x31;
extern uint8_t vector_handler_0x32;
extern uint8_t vector_handler_0x33;
extern uint8_t vector_handler_0x34;
extern uint8_t vector_handler_0x35;
extern uint8_t vector_handler_0x36;
extern uint8_t vector_handler_0x37;
extern uint8_t vector_handler_0x38;
extern uint8_t vector_handler_0x39;
extern uint8_t vector_handler_0x3A;
extern uint8_t vector_handler_0x3B;
extern uint8_t vector_handler_0x3C;
extern uint8_t vector_handler_0x3D;
extern uint8_t vector_handler_0x3E;
uint64_t hpet_vec_handler[32] = {
    (uint64_t)&vector_handler_0x30,
    (uint64_t)&vector_handler_0x31,
    (uint64_t)&vector_handler_0x32,
    (uint64_t)&vector_handler_0x33,
    (uint64_t)&vector_handler_0x34,
    (uint64_t)&vector_handler_0x35,
    (uint64_t)&vector_handler_0x36,
    (uint64_t)&vector_handler_0x37,
    (uint64_t)&vector_handler_0x38,
    (uint64_t)&vector_handler_0x39,
    (uint64_t)&vector_handler_0x3A,
    (uint64_t)&vector_handler_0x3B,
    (uint64_t)&vector_handler_0x3C,
    (uint64_t)&vector_handler_0x3D,
    (uint64_t)&vector_handler_0x3E
};

volatile uint64_t *hpet_base_addr;
uint32_t hpet_period;
uint8_t timer_count;
bool hpet_available = false;

uint64_t read_hpet_register(uint16_t reg) {
    return *(hpet_base_addr + reg);
}
void write_hpet_register(uint16_t reg, uint64_t value) {
    *(hpet_base_addr + reg) = value;
}

void enable_hpet() {
    uint64_t config = read_hpet_register(GENERAL_CONFIGURATION_REG);
    config |= 1;
    write_hpet_register(GENERAL_CONFIGURATION_REG, config);
}

void configure_timer(uint8_t timer, timer_config_t conf) {
    // Timer is not available
    if (timer >= timer_count) return;

    write_hpet_register(TIMER_CONFIG_REG(timer), conf.raw);
}

uint64_t poll_hpet() {
    return read_hpet_register(MAIN_COUNTER_VALUE_REG);
}

void sleep_polled_hpet(uint64_t femto) {
    uint64_t cycle = femto / hpet_period;
    uint64_t main_counter = poll_hpet();
    uint64_t target = main_counter + cycle;

    if (main_counter > target)
        while(poll_hpet() > target) {}

    while(poll_hpet() < target) {}
    return;
}

void nanodelay(uint64_t nano) { sleep_polled_hpet(nano * 10e6); }

void millidelay(uint64_t milli) {
    nanodelay(milli * 10e6);
}

int arm_hpet_timer(uint8_t timer, uint64_t femto, bool periodic) {
    if (timer >= timer_count) return 1;
    uint64_t cycle = femto / (uint64_t) hpet_period;
    timer_config_t config = (timer_config_t) read_hpet_register(TIMER_CONFIG_REG(timer));

    if (config.int_route_cnf == 0) {
        uint8_t irq_used = 9;
        uint32_t available_irq = config.int_route_cap;
        available_irq >>=9;
        while (available_irq) {

            while ((available_irq & 1) == 0) {
                irq_used ++;
                available_irq >>= 1;
            }
            if (!set_irq(irq_used, 0x30 + timer, 0, false)) break;
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
	if (periodic)
        write_hpet_register(TIMER_COMPARATOR_REG(timer), cycle);

    return 0;
}

int setup_hpet() {
    acpi_sdt_header_t *header = find_table("HPET");
    if (!validate_sdt(header)) return 1;

    hpet_acpi_t *hpet_table = (hpet_acpi_t *) header;

    hpet_base_addr = map_mmio(&kernel_vmm, hpet_table->base_addr.addr, 0x400, true);
    if (hpet_base_addr == NULL) {
        logf(ERROR, "MMIO mapping for hpet failed");
        return 1;
    }

    uint64_t general_cap = read_hpet_register(GENERAL_CAPABILTIES_REG);
    timer_count = ((general_cap >> 8) & 0x1F) + 1;
    hpet_period = (uint32_t)(general_cap >> 32);
    logf(INFO, "HPET found with %d timer available", timer_count);

    enable_hpet();

    for (uint8_t i = 0; i < timer_count; i ++) {
        set_idt_entry(HPET_BASE_INT_VEC + i, hpet_vec_handler[i],
                      GDT_ENTRY_KERNEL_CODE,
                      FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE)
            );

    }

    return 0;
}
