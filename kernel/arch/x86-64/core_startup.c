#include <cpuid.h>
#include <kernel/acpi.h>
#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/apic_timer.h>
#include <kernel/arch/x86-64/hpet.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/arch/x86-64/ioapic.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/debug/qemu.h>
#include <kernel/kernel.h>
#include <kernel/keyboard.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vm.h>
#include <kernel/multiboot2.h>
#include <kernel/sync.h>
#include <kernel/timer.h>
#include <kernel/tty.h>
#include <kernel/x86-64.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint8_t ap_trampoline;
extern uint8_t vector_handler_0x21;
spinlock_t core_running_lock = {.name = "Core Running"};

void init_ap() {
    // load ap_trampoline at 0x8000 (physical)
    memcpy((void *)(0x8000 + PHYSICAL_OFFSET), &ap_trampoline, 4096);

    acquire(&core_running_lock);
    kernel_status.core_running++;
    for (uint32_t i = 0; i < kernel_status.core_available; i++) {
        // do not start BSP, that's already running this code
        if (i == kernel_status.bsp_id) continue;
        // send INIT IPI
        write_lapic_register(LAPIC_REG_ERROR_STATUS, 0);
        ipi_command_t ini = {
            .deliv_mode = DELIVERY_INIT,
            .level = 1,
            .trigger_mode = 1,
        };
        send_ipi(i, ini);

        // send INIT Deassert
        ini.deliv_mode = DELIVERY_INIT;
        ini.level = 0;
        send_ipi(i, ini);

        millidelay(10);  // wait 10 msec

        // send STARTUP IPI (twice)
        ipi_command_t startup = {
            .deliv_mode = DELIVERY_START_UP,
            .vector = 8,
        };
        for (uint8_t j = 0; j < 2; j++) {
            write_lapic_register(LAPIC_REG_ERROR_STATUS, 0);
            send_ipi(i, startup);
            nanodelay(200e3);  // wait 200 usec
        }
    }

    release(&core_running_lock);
    millidelay(20);
    logf(INFO, "After startup there are %d CPU running",
         kernel_status.core_running);
}

void ap_startup(uint32_t apicid) {
    push_off();
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    uint32_t nx_flag_supported = (edx & (1L << 20)) != 0;
    if (kernel_status.nx_flag_enabled) {
        if (!nx_flag_supported) {
            panic(
                "Error while setting up CPU %d, it does not suppord nx_flag "
                "but nx_flag is enabled on BPS");
        } else {
            enable_nx_flag();
        }
    }

    load_idt();
    enable_lapic(apicid);

    acquire(&core_running_lock);
    kernel_status.core_running++;
    release(&core_running_lock);

    pop_off();
    main();
}

void bsp_startup(unsigned long magic, unsigned long addr, uint32_t apicid) {
    push_off();
    init_qemu_serial();
    kernel_status.bsp_id = apicid;
    logf(INFO, "BSP id is %d", kernel_status.bsp_id);
    struct multiboot_memory_map *memory_map = NULL;
    struct multiboot_framebuffer *framebuffer = NULL;
    struct multiboot_acpi_old *acpi_old = NULL;
    struct multiboot_acpi_new *acpi_new = NULL;

    struct multiboot_boot_information boot_info = {.memory_map = &memory_map,
                                                   .framebuffer = &framebuffer,
                                                   .acpi_old = &acpi_old,
                                                   .acpi_new = &acpi_new};

    // addr is a physical addr but the identity mapping
    // used for the long jump is still active
    load_multiboot_info(magic, addr, &boot_info);

    kvminit(memory_map);

    if (acpi_old) load_rsdp((void *)&acpi_old->rsdp + PHYSICAL_OFFSET);

    if (acpi_new) load_xsdp((void *)&acpi_new->rsdp + PHYSICAL_OFFSET);

    load_idt();  // Setup interrupts
    enable_lapic(kernel_status.bsp_id);
    read_madt();

    setup_hpet();

    init_ap();

    disable_id_mapping();

    // Enable keyboard support
    init_keyboard();
    set_idt_entry(IRQ_VECTOR_KEYBOARD, (uint64_t)&vector_handler_0x21,
                  GDT_ENTRY_KERNEL_CODE, FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    set_irq(IRQ_KEYBOARD, IRQ_VECTOR_KEYBOARD, DEST_PHYSICAL, 0, false);

    memory_area_t *area = vmm_alloc_at(BIG_PAGE_SIZE, &kernel_vmm,
                                       2 * PAGE_SIZE, MEMORY_FLAG_WRITE);
    if (!area) panic("Could not allocate memory for kernel heap");
    kernel_heap = (heap_node_t *)area->start;
    init_heap(kernel_heap, area->size);

    terminal_initialize((void *)&framebuffer->fb + PHYSICAL_OFFSET);

    pop_off();
    main();
}
