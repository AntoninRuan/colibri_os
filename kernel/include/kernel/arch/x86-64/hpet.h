#ifndef HPET_H
#define HPET_H

#include <kernel/acpi.h>
#include <stdint.h>

struct generic_address {
    u8 addr_space_id;
    u8 reg_bit_width;
    u8 reg_bit_offset;
    u8 reserved;
    u64 addr;
} __attribute__((packed));

struct timer_block_id {
    u8 hardware_rev_id;         // Bit 0..7
    u8 comparator_cound   : 5;  // Bit 8..12
    u8 count_size_cap     : 1;  // Bit 13
    u8 reserved           : 1;  // Bit 14
    u8 legacy_replacement : 1;  // Bit 15
    u16 pci_vendor_id;
} __attribute__((packed));

struct hpet_acpi {
    acpi_sdt_header_t header;

    struct timer_block_id timer_block_id;
    struct generic_address base_addr;
    u8 hpet_number;
    u16 min_clock_tick;
    u8 attributes;
} __attribute__((packed));

typedef struct hpet_acpi hpet_acpi_t;

union timer_config {
    struct {
        u8                 : 1;  // Bit 0
        u8 int_type_cnf    : 1;  // Bit 1
        u8 int_enb_cnf     : 1;  // Bit 2
        u8 per_int_cnf     : 1;  // Bit 3
        u8 per_int_cap     : 1;  // Bit 4
        u8 size_cap        : 1;  // Bit 5
        u8 val_set_cnf     : 1;  // Bit 6
        u8                 : 1;  // Bit 7
        u8 mode32_cnf      : 1;  // Bit 8
        u8 int_route_cnf   : 5;  // Bit 9..13
        u8 fsb_en_cnf      : 1;  // Bit 14
        u8 fsb_int_del_cap : 1;  // Bit 15
        u16                : 0;
        u32 int_route_cap;
    };
    u64 raw;
} __attribute__((packed));

typedef union timer_config timer_config_t;

#define GENERAL_CAPABILTIES_REG   (0)
#define GENERAL_CONFIGURATION_REG (2)
#define GENERAL_STATUS_REG        (4)
#define MAIN_COUNTER_VALUE_REG    (30)
#define TIMER_CONFIG_REG(n)       (32 + (4 * n))
#define TIMER_COMPARATOR_REG(n)   (33 + (4 * n))
#define TIMER_INTERRUPT_REG(n)    (34 + (4 * n))

#define HPET_BASE_INT_VEC 0x30

void sleep_polled_hpet(u64 femto);
int arm_hpet_timer(u8 timer, u64 femto, bool periodic);
int setup_hpet();

#endif  // HPET_H
