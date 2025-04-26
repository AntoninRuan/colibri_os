#include <cpuid.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/acpi.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/x86-64.h>

#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/interrupt.h>

volatile void* lapic_base_address[MAX_CORES];
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

bool check_lapic_availability(uint32_t apic_id) {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    bool has_lapic = (edx & 0x100);

    // Also check for xapic2 support
    support_xapic2[apic_id] = (ecx & (1 << 21));

    return has_lapic;
}

uint32_t read_lapic_register(uint32_t reg) {
    uint32_t apic_id = get_apic_id();
    uint32_t value;
    if (support_xapic2[apic_id]) {
        uint32_t apic_reg_msr = 0x800 + (reg >> 4);
        value = rdmsr(apic_reg_msr);
    } else {
        uint32_t *index = (uint32_t *)(lapic_base_address[apic_id] + reg);
        value = *index;
    }
    return value;
}

void write_lapic_register(uint32_t reg, uint32_t value) {
    uint32_t apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        uint32_t apic_reg_msr = 0x800 + (reg >> 4);
        wrmsr(apic_reg_msr, (uint64_t)value);
    } else {
        uint32_t *index = (uint32_t *)(lapic_base_address[apic_id] + reg);
        *index = value;
    }
}

uint64_t read_icr() {
    uint32_t apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        return rdmsr(0x830);
    } else {
        uint32_t low = read_lapic_register(LAPIC_REG_INTERRUPT_COMMAND);
        uint32_t high = read_lapic_register(LAPIC_REG_INTERRUPT_COMMAND + 0x10);
        return (uint64_t)low | ((uint64_t)high << 32);
    }
}

void write_icr(uint64_t value) {
    uint32_t apic_id = get_apic_id();
    if (support_xapic2[apic_id]) {
        wrmsr(0x830, value);
    } else {
        uint32_t low = (uint32_t)value;
        uint32_t high = (uint32_t)(value >> 32);
        write_lapic_register(LAPIC_REG_INTERRUPT_COMMAND + 0x10, high);
        write_lapic_register(LAPIC_REG_INTERRUPT_COMMAND, low);
    }
}

void send_ipi(uint32_t dest, ipi_command_t command) {
    uint32_t apic_id = get_apic_id();
    uint64_t full_cmd = (uint64_t) command.raw;
    if(support_xapic2[apic_id]) {
        full_cmd |= (uint64_t) dest << 32;
    } else {
        full_cmd |= (uint64_t) dest << 56;
    }
    write_icr(full_cmd);

    if(!support_xapic2[apic_id]) {
        do {
            __asm_pause();
        } while (((ipi_command_t)(uint32_t)read_icr()).deliv_status); // wait for delivery
    }

    return;
}

void send_eoi() { write_lapic_register(LAPIC_REG_EOI, 0); }

// Defined in isr_wrapper.S
extern uint8_t vector_handler_0xFF;

int enable_lapic(uint32_t apic_id) {
    if (!check_lapic_availability(apic_id))
        return 1;

    disable_pic();

    uint64_t apic_base = rdmsr(IA32_APIC_BASE);

    apic_base |= 0x800; // Make sure lapic is enabled

    if (support_xapic2[apic_id]) { // Always enable xapic2 if it is supported
        apic_base |= (1 << 10);
    }

    wrmsr(IA32_APIC_BASE, apic_base);

    if (!support_xapic2[apic_id]) {
        // 0x400 is the size of all lapic registers
        lapic_base_address[apic_id] =
            map_mmio(NULL, apic_base & 0xFFFFF000, 0x400, true);
        if (lapic_base_address[apic_id] == NULL) {
            logf(ERROR, "MMIO mapping for lapic registers failed for cpu %d", apic_id);
            return 1;
        }
    }

    uint32_t spurious_vector = 0xFF | 0x100;

    write_lapic_register(LAPIC_REG_SPURIOUS_VECTOR, spurious_vector);

    set_idt_entry(0xFF, (uint64_t)&vector_handler_0xFF, GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    if (support_xapic2[apic_id])
        logf(INFO, "LAPIC for CPU %d is enabled with xapic2 support", apic_id);
    else
        logf(INFO, "LAPIC for CPU %d is enabled without xapic2 support",
             apic_id);

    return 0;
}
