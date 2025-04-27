#ifndef APIC_TIMER_H
#define APIC_TIMER_H

#include <stdbool.h>
#include <stdint.h>

void arm_apic_timer(uint64_t nano, bool periodic);

#endif  // APIC_TIMER_H
