#include <kernel/log.h>
#include <math.h>
#include <stdbool.h>

#include <kernel/kernel.h>
#include <kernel/synchronization/spinlock.h>

spinlock_t spinlocks[512] = {0};

bool holding(spinlock_t *lock) {
    return (lock->held && lock->cpu_id == get_cpu_id());
}

void acquire(spinlock_t *lock) {
    disable_interrupt();

    if (holding(lock)) {
        logf(ERROR, "CPU %d is already holding lock", get_cpu_id());
        panic("acquire");
    }

#define WARNING 10000000
    uint64_t nb_tries = 0;
    bool warned = false;
    while(__sync_lock_test_and_set(&lock->held, 1) != 0) {
        if (nb_tries > WARNING && !warned) {
            logf(WARNING, "CPU %d is waiting for a lock", get_cpu_id());
            warned = true;
        }
        nb_tries ++;
    }

    lock->cpu_id = get_cpu_id();
}

void release(spinlock_t *lock) {
    disable_interrupt();

    lock->cpu_id = 0;

    __sync_synchronize();

    __sync_lock_release(&lock->held);

    enable_interrupt();
}
