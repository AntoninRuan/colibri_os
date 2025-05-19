#include <kernel/acpi.h>
#include <kernel/arch/i386/ioapic.h>
#include <stddef.h>
#include <stdint.h>

u32 io_apic_base_addr = 0;

u32 read_ioapic_register(u8 reg) {
    *(u32 *)io_apic_base_addr = (u32)reg;
    return *(u32 *)(io_apic_base_addr + 0x10);
}

void write_ioapic_register(u8 reg, u32 value) {
    if (io_apic_base_addr == 0) return;

    *(u32 *)io_apic_base_addr = (u32)reg;
    *(u32 *)(io_apic_base_addr + 0x10) = value;
}

void write_ioapic_redirect(u8 index, io_apic_redirect_entry_t entry) {
    if (index < 0x10 || index > 0x3F) return;
    if (index % 2) return;

    u32 low = (u32)(entry.raw & 0xFFFFFFFF);
    u32 high = (u32)(entry.raw >> 32);

    write_ioapic_register(index, low);
    write_ioapic_register(index + 1, high);
}

void set_irq(u8 irq, u8 idt_entry, u32 flags, bool masked) {
    io_apic_redirect_entry_t entry;
    entry.raw = flags | (idt_entry & 0xFF);

    // TODO Override detection
    // TODO multiple ioapic handling

    entry.masked = masked;
    write_ioapic_redirect(0x10 + (irq * 2), entry);
}

int read_madt() {
    struct madt *madt = (struct madt *)find_table(ACPI_TABLE_APIC);
    if (madt == NULL) return 1;

    struct ic_headers *header;
    for (header = (struct ic_headers *)madt->interrupt_controllers;
         ((u32)header) - ((u32)madt) < madt->header.length;
         header = (struct ic_headers *)((u8 *)header + header->length)) {
        switch (header->type) {
            case IC_TYPE_IO_APIC:
                struct ic_io_apic *ioapic = (struct ic_io_apic *)header;
                if (io_apic_base_addr == 0) io_apic_base_addr = ioapic->address;
                (void)ioapic;
                break;

            default:
                break;
        }
    }

    return 0;
}
