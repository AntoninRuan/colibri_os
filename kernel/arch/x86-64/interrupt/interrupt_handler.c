#include <kernel/arch/x86-64/apic.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/keyboard.h>
#include <kernel/log.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler.h>
#include <kernel/x86-64.h>

extern void *interrupt_dispatch(struct interrupt_frame *context) {
    switch (context->vector_number) {
        case GENERAL_PROTECTION:
            panic("#GP");
            break;

        case PAGE_FAULT:
            pg_error_t error = {0};
            error.raw = context->error_code;
            if (!(error.present || error.rsvd || error.ifetch || error.pk ||
                  error.ss || error.hlat || error.sgx)) {
                if (on_demand_allocation((void *)context->registers.cr2)) {
                    panic("Error for on demand allocation, memory not allocated");
                }
            } else {
                panic("Unresolvable #PF");
            }
            break;

        case IRQ_VECTOR_KEYBOARD:
            keystroke_handler();
            break;

        case IRQ_HPET_TIMER(0):
            log(DEBUG, "PING (hpet)");
            break;

        case APIC_TIMER_INTERRUPT:
            log(DEBUG, "PING (apic)");
            schedule(context);
            break;

        default:
            break;
    }

    send_eoi();
    return context;
}
