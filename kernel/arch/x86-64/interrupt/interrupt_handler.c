#include <kernel/keyboard.h>
#include <kernel/x86.h>
#include <kernel/arch/i386/apic.h>


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
