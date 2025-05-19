#include <cpuid.h>
#include <kernel/acpi.h>
#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/x86-64.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

volatile void *lapic_base_address[MAX_CORES];
bool support_xapic2[MAX_CORES] = {0};

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

bool check_lapic_availability(u32 apic_id) {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    bool has_lapic = (edx & 0x100);

    // Also check for xapic2 support
    support_xapic2[apic_id] = (ecx & (1 << 21));

    return has_lapic;
}

u32 read_lapic_register(u32 reg) {
    u32 apic_id = get_apic_id();
    u32 value;
    if (support_xapic2[apic_id]) {
        u32 apic_reg_msr = 0x800 + (reg >> 4);
        value = rdmsr(apic_reg_msr);
    } else {
        u32 *index = (u32 *)(lapic_base_address[apic_id] + reg);
        value = *index;
    }
    return value;
}

void write_lapic_register(u32 reg, u32 value) {
    u32 apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        u32 apic_reg_msr = 0x800 + (reg >> 4);
        wrmsr(apic_reg_msr, (u64)value);
    } else {
        u32 *index = (u32 *)(lapic_base_address[apic_id] + reg);
        *index = value;
    }
}

u64 read_icr() {
    u32 apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        return rdmsr(0x830);
    } else {
        u32 low = read_lapic_register(LAPIC_REG_INTERRUPT_COMMAND);
        u32 high = read_lapic_register(LAPIC_REG_INTERRUPT_COMMAND + 0x10);
        return (u64)low | ((u64)high << 32);
    }
}

void write_icr(u64 value) {
    u32 apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        wrmsr(0x830, value);
    } else {
        u32 low = (u32)value;
        u32 high = (u32)(value >> 32);
        write_lapic_register(LAPIC_REG_INTERRUPT_COMMAND + 0x10, high);
        write_lapic_register(LAPIC_REG_INTERRUPT_COMMAND, low);
    }
}

void send_ipi(u32 dest, ipi_command_t command) {
    u32 apic_id = get_apic_id();
    u64 full_cmd = (u64)command.raw;
    if (support_xapic2[apic_id]) {
        full_cmd |= (u64)dest << 32;
    } else {
        full_cmd |= (u64)dest << 56;
    }
    write_icr(full_cmd);

    if (!support_xapic2[apic_id]) {
        do {
            __asm_pause();
        } while (((ipi_command_t)(u32)read_icr())
                     .deliv_status);  // wait for delivery
    }

    return;
}

void send_eoi() { write_lapic_register(LAPIC_REG_EOI, 0); }

// Defined in isr_wrapper.S
extern u8 vector_handler_0xFF;

int enable_lapic(u32 apic_id) {
    if (!check_lapic_availability(apic_id)) return 1;

    disable_pic();

    u64 apic_base = rdmsr(IA32_APIC_BASE);

    apic_base |= 0x800;  // Make sure lapic is enabled

    if (support_xapic2[apic_id]) {  // Always enable xapic2 if it is supported
        apic_base |= (1 << 10);
    }

    wrmsr(IA32_APIC_BASE, apic_base);

    if (!support_xapic2[apic_id]) {
        // 0x400 is the size of all lapic registers
        lapic_base_address[apic_id] =
            map_mmio(NULL, apic_base & 0xFFFFF000, 0x400, true);
        if (lapic_base_address[apic_id] == NULL) {
            logf(ERROR, "MMIO mapping for lapic registers failed for cpu %d",
                 apic_id);
            return 1;
        }
    }

    u32 spurious_vector = 0xFF | 0x100;

    write_lapic_register(LAPIC_REG_SPURIOUS_VECTOR, spurious_vector);

    set_idt_entry(0xFF, (u64)&vector_handler_0xFF, GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    if (support_xapic2[apic_id])
        logf(INFO, "LAPIC for CPU %d is enabled with xapic2 support", apic_id);
    else
        logf(INFO, "LAPIC for CPU %d is enabled without xapic2 support",
             apic_id);

    return 0;
}
