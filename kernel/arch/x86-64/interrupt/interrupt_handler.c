#include <kernel/keyboard.h>
#include <kernel/x86-64.h>
#include <kernel/arch/x86-64/apic.h>

extern void *interrupt_dispatch(struct interrupt_frame *context) {
    switch (context->vector_number) {
        case GENERAL_PROTECTION:
        case PAGE_FAULT:
            while(1);
            break;
        case IRQ_VECTOR_KEYBOARD:
            keystroke_handler();
            break;
        default:
            break;
    }

    send_eoi();
    return context;
}
