#ifndef IOAPIC_H
#define IOAPIC_H

#include <kernel/acpi.h>

#define IC_TYPE_LAPIC                     0
#define IC_TYPE_IO_APIC                   1
#define IC_TYPE_INTERRUPT_SOURCE_OVERRIDE 2
#define IC_TYPE_NMI_SOURCE                3
#define IC_TYPE_LAPIC_NMI                 4
#define IC_TYPE_LAPIC_ADDR_OVERRIDE       5
#define IC_TYPE_IO_SAPIC                  6
#define IC_TYPE_LOCAL_SAPIC               7
#define IC_TYPE_PLATFORM_INTERRUPT_SOURCE 8
#define IC_TYPE_PROC_LOCAL_X2APIC         9
#define IC_TYPE_LOCAL_X2APIC_NMI          10
#define IC_TYPE_GIC_CPU_INTERFACE         11
#define IC_TYPE_GIC_DISTRIBUTOR           12
#define IC_TYPE_GIC_MSI_FRAME             13
#define IC_TYPE_GIC_REDISTRIBUTOR         14
#define IC_TYPE_GIC_ITS                   15

struct ic_headers {
    uint8_t type;
    uint8_t length;
};

struct ic_io_apic {
    struct ic_headers header;

    uint8_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed));

struct madt {
    struct acpi_sdt_header header;

    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t interrupt_controllers[];
} __attribute__((packed));

typedef union io_apic_redirect_entry_t {
    struct {
        uint64_t vector           :8;  // bit 0..7
        uint64_t delivery_mode    :3;  // bit 8..10
        uint64_t destination_mode :1;  // bit 11
        uint64_t delivery_status  :1;  // bit 12
        uint64_t intpol           :1;  // bit 13
        uint64_t remote_irr       :1;  // bit 14
        uint64_t trigger_mode     :1;  // bit 15
        uint64_t masked           :1;  // bit 16
        uint64_t reserved         :39; // 17..55
        uint64_t destination      :8;  // 56..64
    };
    uint64_t raw;
} io_apic_redirect_entry_t;

int read_madt();
void set_irq(uint8_t irq, uint8_t idt_entry, uint32_t flags, bool masked);

#endif // IOAPIC_H
