#include <kernel/acpi.h>
#include <kernel/arch/x86-64/ioapic.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/x86-64.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

volatile u32 *io_apic_reg_sel = 0;
volatile u32 *io_apic_win = 0;

u32 read_ioapic_register(u8 reg) {
    *io_apic_reg_sel = (u32)reg;
    return *io_apic_win;
}

void write_ioapic_register(u8 reg, u32 value) {
    if (io_apic_reg_sel == 0) return;

    *io_apic_reg_sel = (u32)reg;
    *io_apic_win = value;
}

void write_ioapic_redirect(u8 index, io_apic_redirect_t entry) {
    if (index < 0x10 || index > 0x3F) return;
    if (index % 2) return;

    u32 low = (u32)(entry.raw & 0xFFFFFFFF);
    u32 high = (u32)(entry.raw >> 32);

    write_ioapic_register(index, low);
    write_ioapic_register(index + 1, high);
}

int set_irq(u8 irq, u8 idt_entry, u8 dest, u8 dest_mode, bool masked) {
    io_apic_redirect_t entry;
    entry.raw = read_ioapic_register(0x10 + (irq * 2));
    if (entry.vector != 0) {
        logf(ERROR, "IRQ %d is already mapped", irq);
        return -1;
    }

    entry.raw = (u64)(idt_entry & 0xFF);

    // TODO multiple ioapic handling

    entry.destination_mode = dest_mode;
    entry.destination = dest;
    entry.masked = masked;
    write_ioapic_redirect(0x10 + (irq * 2), entry);
    return 0;
}

int read_madt() {
    struct madt *madt = (struct madt *)find_table(ACPI_TABLE_APIC);
    if (madt == NULL) return 1;

    struct ic_headers *header;
    for (header = (struct ic_headers *)madt->interrupt_controllers;
         ((u64)header) - ((u64)madt) < madt->header.length;
         header = (struct ic_headers *)((void *)header + header->length)) {
        switch (header->type) {
            case IC_TYPE_LAPIC:
                ic_lapic_t *lapic = (ic_lapic_t *)header;
                if (lapic->flags & 1) {
                    kernel_status.core_available++;
                }
                break;
            case IC_TYPE_IO_APIC:
                struct ic_io_apic *ioapic = (struct ic_io_apic *)header;
                if (io_apic_reg_sel == 0) {
                    io_apic_reg_sel =
                        map_mmio(NULL, ioapic->address, 0x20, true);
                    if (io_apic_reg_sel == NULL) {
                        logf(ERROR,
                             "MMIO Mapping for io_apic registers failed");
                        return 1;
                    }
                    io_apic_win = (u32 *)((uintptr_t)io_apic_reg_sel + 0x10);
                }
                break;

            default:
                break;
        }
    }

    if (kernel_status.core_available > MAX_CORES)
        kernel_status.core_available = MAX_CORES;

    return 0;
}
