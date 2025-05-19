#ifndef APIC_TIMER_H
#define APIC_TIMER_H

#include <stdbool.h>
#include <sys/cdefs.h>

void arm_apic_timer(u64 nano, bool periodic);

#endif  // APIC_TIMER_H
