#include <kernel/keyboard.h>

#include "interrupt.h"
#include "apic.h"

extern void *interrupt_dispatch(struct interrupt_frame *context) {
    switch (context->vector_number) {
        case IRQ_VECTOR_KEYBOARD:
            keystroke_handler();
            break;
        default:
            break;
    }

    send_eoi();
    return context;
}
