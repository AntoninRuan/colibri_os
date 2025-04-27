#include <kernel/arch/x86-64/apic_timer.h>
#include <kernel/arch/x86-64/hpet.h>
#include <kernel/timer.h>

void nanodelay(uint64_t nano) { sleep_polled_hpet(nano * 1e6); }

void millidelay(uint64_t milli) { nanodelay(milli * 1e6); }

void arm_timer(uint64_t nano, bool periodic, bool local) {
    if (local)
        arm_apic_timer(nano, periodic);
    else
        arm_hpet_timer(0, nano * 1e6, periodic);
}
