#include <kernel/arch/x86-64/apic_timer.h>
#include <kernel/arch/x86-64/hpet.h>
#include <kernel/timer.h>

void nanodelay(u64 nano) { sleep_polled_hpet(nano * 1e6); }

void millidelay(u64 milli) { nanodelay(milli * 1e6); }

void arm_timer(u64 nano, bool periodic, bool local) {
    if (local)
        arm_apic_timer(nano, periodic);
    else
        arm_hpet_timer(0, nano * 1e6, periodic);
}
