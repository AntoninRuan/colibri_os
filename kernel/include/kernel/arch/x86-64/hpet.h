#ifndef HPET_H
#define HPET_H

#include <stdint.h>
#include <kernel/acpi.h>

struct generic_address {
    uint8_t addr_space_id;
    uint8_t reg_bit_width;
    uint8_t reg_bit_offset;
    uint8_t reserved;
    uint64_t addr;
} __attribute__((packed));

struct timer_block_id {
    uint8_t hardware_rev_id;       // Bit 0..7
    uint8_t comparator_cound  : 5; // Bit 8..12
    uint8_t count_size_cap    : 1; // Bit 13
    uint8_t reserved          : 1; // Bit 14
    uint8_t legacy_replacement :1; // Bit 15
    uint16_t pci_vendor_id;
} __attribute__((packed));

struct hpet_acpi {
    acpi_sdt_header_t header;

    struct timer_block_id timer_block_id;
    struct generic_address base_addr;
    uint8_t hpet_number;
    uint16_t min_clock_tick;
    uint8_t attributes;
} __attribute__((packed));

typedef struct hpet_acpi hpet_acpi_t;

union timer_config {
    struct {
        uint8_t                 :1; // Bit 0
        uint8_t int_type_cnf    :1; // Bit 1
        uint8_t int_enb_cnf     :1; // Bit 2
        uint8_t per_int_cnf     :1; // Bit 3
        uint8_t per_int_cap     :1; // Bit 4
        uint8_t size_cap        :1; // Bit 5
        uint8_t val_set_cnf     :1; // Bit 6
        uint8_t                 :1; // Bit 7
        uint8_t mode32_cnf      :1; // Bit 8
        uint8_t int_route_cnf   :5; // Bit 9..13
        uint8_t fsb_en_cnf      :1; // Bit 14
        uint8_t fsb_int_del_cap :1; // Bit 15
        uint16_t                :0;
        uint32_t int_route_cap;
    };
    uint64_t raw;
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

int setup_hpet();

#endif // HPET_H
