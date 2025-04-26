#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/sync.h>
#include <math.h>
#include <stdbool.h>

spinlock_t spinlocks[512] = {0};

bool holding(spinlock_t *lock) {
    return (lock->held && lock->cpu_id == get_cpu_id());
}

void acquire(spinlock_t *lock) {
    disable_interrupt();

    if (holding(lock)) {
        logf(ERROR, "CPU %d is already holding lock %s", get_cpu_id(), lock->name);
        panic("acquire");
    }

#define WARNING 10000000
    uint64_t nb_tries = 0;
    bool warned = false;
    while (__atomic_test_and_set(&lock->held, __ATOMIC_ACQUIRE)) {
        if (nb_tries > WARNING && !warned) {
            logf(WARNING, "CPU %d is waiting for lock %s", get_cpu_id(), lock->name);
            warned = true;
        }
        nb_tries++;
    }

    lock->cpu_id = get_cpu_id();
}

void release(spinlock_t *lock) {
    disable_interrupt();

    lock->cpu_id = 0;

    __atomic_thread_fence(__ATOMIC_ACQ_REL);

    __atomic_clear(&lock->held, __ATOMIC_RELEASE);

    enable_interrupt();
}
