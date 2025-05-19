#include <cpuid.h>
#include <kernel/acpi.h>
#include <kernel/arch/i386/apic.h>
#include <kernel/arch/i386/interrupt.h>
#include <kernel/x86.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

u32 lapic_base_address = 0;
bool support_xapic2 = false;

void disable_pic() {
    outb(PIC_COMMAND_MASTER, ICW_1);
    outb(PIC_COMMAND_SLAVE, ICW_1);
    iowait();
    outb(PIC_DATA_MASTER, ICW_2_M);
    outb(PIC_DATA_SLAVE, ICW_2_S);
    iowait();
    outb(PIC_DATA_MASTER, ICW_3_M);
    outb(PIC_DATA_SLAVE, ICW_3_S);
    iowait();
    outb(PIC_DATA_MASTER, ICW_4_8086);
    outb(PIC_DATA_SLAVE, ICW_4_8086);
    iowait();

    // Masking all interrupts
    outb(PIC_DATA_MASTER, 0xFF);
    outb(PIC_DATA_SLAVE, 0xFF);
}

bool check_lapic_availability() {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    bool has_lapic = (edx & 0x100);

    // Also check for xapic2 support
    support_xapic2 = (ecx & (1 << 21));

    return has_lapic;
}

u32 read_lapic_register(u32 reg) {
    u32 value;
    if (support_xapic2) {
        u32 apic_reg_msr = 0x800 + (reg >> 4);
        value = rdmsr(apic_reg_msr);
    } else {
        u32 *index = (u32 *)(lapic_base_address + reg);
        value = *index;
    }
    return value;
}

void write_lapic_register(u32 reg, u32 value) {
    if (support_xapic2) {
        u32 apic_reg_msr = 0x800 + (reg >> 4);
        wrmsr(apic_reg_msr, (u64)value);
    } else {
        u32 *index = (u32 *)(lapic_base_address + reg);
        *index = value;
    }
}

void send_eoi() { write_lapic_register(LAPIC_REGISTER_EOI, 0); }

// Defined in isr_wrapper.S
extern u8 vector_handler_0xFF;

int enable_lapic() {
    if (!check_lapic_availability()) return 1;

    disable_pic();

    u64 apic_base = rdmsr(IA32_APIC_BASE);

    apic_base |= 0x800;  // Make sure lapic is enabled

    if (support_xapic2) {  // Always enable xapic2 if it is supported
        apic_base |= (1 << 10);
    }

    wrmsr(IA32_APIC_BASE, apic_base);

    lapic_base_address = (apic_base & 0xFFFFF000);  // Only used if using xapic

    u32 spurious_vector = 0xFF | 0x100;

    write_lapic_register(LAPIC_REGISTER_SPURIOUS_VECTOR, spurious_vector);

    set_idt_entry(0xFF, (u32)&vector_handler_0xFF, GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    return 0;
}
