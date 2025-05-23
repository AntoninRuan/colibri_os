#ifndef APIC_H
#define APIC_H

#include <cpuid.h>
#include <sys/cdefs.h>
#define PIC_COMMAND_MASTER 0x20
#define PIC_DATA_MASTER    0x21

#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_SLAVE    0xA1

#define ICW_1      0x11
#define ICW_2_M    0x20  // Vector offset for master
#define ICW_2_S    0x28  // Vector offst for slave
#define ICW_3_M    0x2
#define ICW_3_S    0x4
#define ICW_4_8086 0x01

#define IA32_APIC_BASE 0x1B

#define LAPIC_REG_ID                   0x20   // R/W
#define LAPIC_REG_VERSION              0x30   // R/W
#define LAPIC_REG_TASK_PRIORITY        0x80   // R/W
#define LAPIC_REG_ARBITRATION_PRIORITY 0x90   // RO
#define LAPIC_REG_PROCESSOR_PRIORITY   0xA0   // RO
#define LAPIC_REG_EOI                  0xB0   // WO
#define LAPIC_REG_REMOTE_READ          0xC0   // RO
#define LAPIC_REG_LOGICAL_DESTINATION  0xD0   // R/W
#define LAPIC_REG_DESTINATION_FORMAT   0xE0   // R/W
#define LAPIC_REG_SPURIOUS_VECTOR      0xF0   // R/W
#define LAPIC_REG_IN_SERVICE           0x100  // RO
#define LAPIC_REG_TRIGGER_MODE         0x180  // RO
#define LAPIC_REG_INTERRUPT_REQUEST    0x200  // RO
#define LAPIC_REG_ERROR_STATUS         0x280  // RO
// LVT Corrected Machine Check Interrupt (CMCI)
#define LAPIC_REG_LVT_CMCI             0x2F0  // R/W
#define LAPIC_REG_INTERRUPT_COMMAND    0x300  // R/W
#define LAPIC_REG_LVT_TIMER            0x320  // R/W
#define LAPIC_REG_LVT_THERMAL          0x330  // R/W
#define LAPIC_REG_LVT_PERFORMANCE      0x340  // R/W
#define LAPIC_REG_LVT_LINT0            0x350  // R/W
#define LAPIC_REG_LVT_LINT1            0x360  // R/W
#define LAPIC_REG_LVT_ERROR            0x370  // R/W
#define LAPIC_REG_INITIAL_COUNT        0x380  // R/W
#define LAPIC_REG_CURRENT_COUNT        0x390  // RO
#define LAPIC_REG_DIVIDE_CONFIG        0x3E0  // R/W

union ipi_command {
    struct {
        u8 vector;  // Bit 0..7
#define DELIVERY_FIXED       0
#define DELIVERY_LOWEST_PRIO 1
#define DELIVERY_SMI         2
#define DELIVERY_NMI         4
#define DELIVERY_INIT        5
#define DELIVERY_START_UP    6
        u8 deliv_mode     : 3;   // Bit 8..10
        u8 dest_mode      : 1;   // Bit 11
        u8 deliv_status   : 1;   // Bit 12
        u8                : 1;   // Bit 13
        u8 level          : 1;   // Bit 14
        u8 trigger_mode   : 1;   // Bit 15
        u8                : 2;   // Bit 16..17
        u8 dest_shorthand : 2;   // Bit 18..19
        u16               : 12;  // Bit 20..31
    };
    u32 raw;
} __attribute__((packed));
typedef union ipi_command ipi_command_t;

static inline u32 get_apic_id() {
    u32 eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ebx >> 24);
}

void send_ipi(u32 dest, ipi_command_t command);
void write_lapic_register(u32 reg, u32 value);
u32 read_lapic_register(u32 reg);

void send_eoi();
int enable_lapic(u32 apicid);

#endif  // APIC_H
