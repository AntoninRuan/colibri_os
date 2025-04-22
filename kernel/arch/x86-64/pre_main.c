#include <stdio.h>

#include <kernel/x86-64.h>
#include <kernel/acpi.h>
#include <kernel/keyboard.h>
#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/multiboot2.h>
#include <kernel/tty.h>

#include <kernel/debug/qemu.h>
#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/arch/x86-64/ioapic.h>

extern uint8_t vector_handler_0x21;

void pre_main(unsigned long magic, unsigned long addr) {
    init_qemu_serial();
    struct multiboot_memory_map *memory_map = NULL;
    struct multiboot_framebuffer *framebuffer = NULL;
    struct multiboot_acpi_old *acpi_old = NULL;
    struct multiboot_acpi_new *acpi_new = NULL;

    struct multiboot_boot_information boot_info = {
        .memory_map = &memory_map,
        .framebuffer = &framebuffer,
        .acpi_old = &acpi_old,
        .acpi_new = &acpi_new
    };

    // addr is a physical addr but the identity mapping
    // used for the long jump is still active
    load_multiboot_info(magic, addr, &boot_info);

    kvminit(memory_map);

    terminal_initialize((void *)&framebuffer->fb + PHYSICAL_OFFSET);
    enable_tty_log();
    log(DEBUG, "TTY log enable");

    if (acpi_old)
        load_rsdp((void *) &acpi_old->rsdp + PHYSICAL_OFFSET);

    if (acpi_new)
        load_xsdp((void *) &acpi_new->rsdp + PHYSICAL_OFFSET);

    load_idt();                 // Setup interrupts
    enable_lapic();
    read_madt();

    // Enable keyboard support
    init_keyboard();
    set_idt_entry(IRQ_VECTOR_KEYBOARD, (uint64_t) &vector_handler_0x21,
                  GDT_ENTRY_KERNEL_CODE,
                  FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE)
        );

    set_irq(IRQ_KEYBOARD, IRQ_VECTOR_KEYBOARD, 0, false);

    asm("sti");
    return;
}
