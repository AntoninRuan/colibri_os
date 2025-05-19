#ifndef IOAPIC_H
#define IOAPIC_H

#include <kernel/acpi.h>
#include <stdint.h>

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
    u8 type;
    u8 length;
};

struct ic_lapic {
    struct ic_headers header;

    u8 acpi_processor_uid;
    u8 apic_id;
    u32 flags;
};
typedef struct ic_lapic ic_lapic_t;

struct ic_io_apic {
    struct ic_headers header;

    u8 id;
    u8 reserved;
    u32 address;
    u32 global_system_interrupt_base;
} __attribute__((packed));

struct madt {
    struct acpi_sdt_header header;

    u32 lapic_addr;
    u32 flags;
    u8 interrupt_controllers[];
} __attribute__((packed));

typedef union io_apic_redirect {
    struct {
        u64 vector        : 8;  // bit 0..7
        u64 delivery_mode : 3;  // bit 8..10
#define DEST_PHYSICAL 0
#define DEST_LOGICAL  1
        u64 destination_mode : 1;   // bit 11
        u64 delivery_status  : 1;   // bit 12
        u64 intpol           : 1;   // bit 13
        u64 remote_irr       : 1;   // bit 14
        u64 trigger_mode     : 1;   // bit 15
        u64 masked           : 1;   // bit 16
        u64 reserved         : 39;  // 17..55
        u64 destination      : 8;   // 56..64
    };
    u64 raw;
} io_apic_redirect_t;

int read_madt();
int set_irq(u8 irq, u8 idt_entry, u8 dest, u8 dest_mode, bool masked);

#endif  // IOAPIC_H
