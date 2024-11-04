#include "interrupt.h"
#include "apic.h"

extern void *interrupt_dispatch(struct interrupt_frame *context) {
    switch (context->vector_number) {
        default:
            break;
    }

    send_eoi();
    return context;
}
