#ifndef APIC_H
#define APIC_H

#include <stdint.h>
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

#define LAPIC_REGISTER_ID                   0x20   // R/W
#define LAPIC_REGISTER_VERSION              0x30   // R/W
#define LAPIC_REGISTER_TASK_PRIORITY        0x80   // R/W
#define LAPIC_REGISTER_ARBITRATION_PRIORITY 0x90   // RO
#define LAPIC_REGISTER_PROCESSOR_PRIORITY   0xA0   // RO
#define LAPIC_REGISTER_EOI                  0xB0   // WO
#define LAPIC_REGISTER_REMOTE_READ          0xC0   // RO
#define LAPIC_REGISTER_LOGICAL_DESTINATION  0xD0   // R/W
#define LAPIC_REGISTER_DESTINATION_FORMAT   0xE0   // R/W
#define LAPIC_REGISTER_SPURIOUS_VECTOR      0xF0   // R/W
#define LAPIC_REGISTER_IN_SERVICE           0x100  // RO
#define LAPIC_REGISTER_TRIGGER_MODE         0x180  // RO
#define LAPIC_REGISTER_INTERRUPT_REQUEST    0x200  // RO
#define LAPIC_REGISTER_ERROR_STATUS         0x280  // RO
// LVT Corrected Machine Check Interrupt (CMCI)
#define LAPIC_REGISTER_LVT_CMCI             0x2F0  // R/W
#define LAPIC_REGISTER_INTERRUPT_COMMAND    0x300  // R/W
#define LAPIC_REGISTER_LVT_TIMER            0x320  // R/W
#define LAPIC_REGISTER_LVT_THERMAL          0x330  // R/W
#define LAPIC_REGISTER_LVT_PERFORMANCE      0x340  // R/W
#define LAPIC_REGISTER_LVT_LINT0            0x350  // R/W
#define LAPIC_REGISTER_LVT_LINT1            0x360  // R/W
#define LAPIC_REGISTER_LVT_ERROR            0x370  // R/W
#define LAPIC_REGISTER_INITIAL_COUNT        0x380  // R/W
#define LAPIC_REGISTER_CURRENT_COUNT        0x390  // RO
#define LAPIC_REGISTER_DIVIDE_CONFIG        0x3E0  // R/W

void send_eoi();
int enable_lapic();

#endif  // APIC_H
