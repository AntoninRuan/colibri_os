#include <cpuid.h>
#include <elf.h>
#include <kernel/acpi.h>
#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/apic_timer.h>
#include <kernel/arch/x86-64/hpet.h>
#include <kernel/arch/x86-64/interrupt.h>
#include <kernel/arch/x86-64/ioapic.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/arch/x86-64/tss.h>
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

extern u8 ap_trampoline;
extern u8 vector_handler_0x21;
spinlock_t core_running_lock = {.name = "Core Running"};

void init_ap() {
    // load ap_trampoline at 0x8000 (physical)
    memcpy((void *)(0x8000 + PHYSICAL_OFFSET), &ap_trampoline, 4096);

    acquire(&core_running_lock);
    kernel_status.core_running++;
    for (u32 i = 0; i < kernel_status.core_available; i++) {
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
        for (u8 j = 0; j < 2; j++) {
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

void ap_startup(u32 apicid) {
    push_off();
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    u32 nx_flag_supported = (edx & (1L << 20)) != 0;
    if (kernel_status.nx_flag_enabled) {
        if (!nx_flag_supported) {
            panic(
                "Error while setting up CPU %d, it does not suppord nx_flag "
                "but nx_flag is enabled on BSP");
        } else {
            enable_nx_flag();
        }
    }

    load_idt();
    enable_lapic(apicid);
    // TODO TSS with SMP
    // setup_tss();

    acquire(&core_running_lock);
    kernel_status.core_running++;
    release(&core_running_lock);

    pop_off();
    main(NULL);
}

void bsp_startup(unsigned long magic, unsigned long addr, u32 apicid) {
    push_off();
    init_qemu_serial();
    kernel_status.bsp_id = apicid;
    logf(INFO, "BSP id is %d", kernel_status.bsp_id);
    struct multiboot_memory_map *memory_map = NULL;
    struct multiboot_framebuffer *framebuffer = NULL;
    struct multiboot_acpi_old *acpi_old = NULL;
    struct multiboot_acpi_new *acpi_new = NULL;
    struct multiboot_module *modules[64] = {0};

    struct multiboot_boot_information boot_info = {.memory_map = &memory_map,
                                                   .framebuffer = &framebuffer,
                                                   .acpi_old = &acpi_old,
                                                   .acpi_new = &acpi_new,
                                                   .module = modules,
                                                   .module_size = 64};

    // addr is a physical addr but the identity mapping
    // used for the long jump is still active
    load_multiboot_info(magic, addr, &boot_info);

    Elf64_Ehdr *initd = NULL;
    for (u64 i = 0; i < boot_info.module_size; i++) {
        if (!strncmp("initd", modules[i]->string, 6)) {
            initd =
                (Elf64_Ehdr *)((u64)modules[i]->mod_start + PHYSICAL_OFFSET);
            break;
        }
    }

    if (initd)
        log(INFO, "Found initd module");
    else
        log(WARNING, "No initd module found");

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
    set_idt_entry(IRQ_VECTOR_KEYBOARD, (u64)&vector_handler_0x21,
                  GDT_ENTRY_KERNEL_CODE, FLAGS_DPL(0) | FLAGS_GATE_TYPE(0xE));

    set_irq(IRQ_KEYBOARD, IRQ_VECTOR_KEYBOARD, DEST_PHYSICAL, 0, false);

    memory_area_t *area =
        vmm_alloc(&kernel_vmm, 2 * PAGE_SIZE, MEMORY_FLAG_WRITE);
    if (!area) panic("Could not allocate memory for kernel heap");
    kernel_heap = (heap_node_t *)area->start;
    init_heap(kernel_heap, area->size);

    setup_tss();

    terminal_initialize((void *)&framebuffer->fb + PHYSICAL_OFFSET);

    pop_off();

    main(initd);
}
