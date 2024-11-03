#include "interrupt.h"

extern void *interrupt_dispatch(struct interrupt_frame *context) {
    switch (context->vector_number) {
        default:
            break;
    }

    return context;
}
