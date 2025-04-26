#include <kernel/acpi.h>
#include <kernel/arch/i386/ioapic.h>
#include <stddef.h>
#include <stdint.h>

uint32_t io_apic_base_addr = 0;

uint32_t read_ioapic_register(uint8_t reg) {
    *(uint32_t *)io_apic_base_addr = (uint32_t)reg;
    return *(uint32_t *)(io_apic_base_addr + 0x10);
}

void write_ioapic_register(uint8_t reg, uint32_t value) {
    if (io_apic_base_addr == 0) return;

    *(uint32_t *)io_apic_base_addr = (uint32_t)reg;
    *(uint32_t *)(io_apic_base_addr + 0x10) = value;
}

void write_ioapic_redirect(uint8_t index, io_apic_redirect_entry_t entry) {
    if (index < 0x10 || index > 0x3F) return;
    if (index % 2) return;

    uint32_t low = (uint32_t)(entry.raw & 0xFFFFFFFF);
    uint32_t high = (uint32_t)(entry.raw >> 32);

    write_ioapic_register(index, low);
    write_ioapic_register(index + 1, high);
}

void set_irq(uint8_t irq, uint8_t idt_entry, uint32_t flags, bool masked) {
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
         ((uint32_t)header) - ((uint32_t)madt) < madt->header.length;
         header = (struct ic_headers *)((uint8_t *)header + header->length)) {
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
