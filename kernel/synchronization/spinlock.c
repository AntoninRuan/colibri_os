#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/sync.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

spinlock_t spinlocks[512] = {0};

bool holding(spinlock_t *lock) {
    return (lock->held && lock->cpu_id == get_cpu()->id);
}

void acquire(spinlock_t *lock) {
    push_off();
    bool log = false;
    if (lock->name)
        log = memcmp(lock->name, "QEMU serial", strlen("QEMU serial"));

    if (holding(lock)) {
        if (log)
            logf(ERROR, "CPU %d is already holding lock %s", get_cpu()->id,
                 lock->name);
        panic("acquire");
    }

#define WARNING 10000000
    u64 nb_tries = 0;
    bool warned = false;
    while (__atomic_test_and_set(&lock->held, __ATOMIC_ACQUIRE)) {
        if (nb_tries > WARNING && !warned) {
            if (log)
                logf(WARNING, "CPU %d is waiting for lock %s", get_cpu()->id,
                     lock->name);
            warned = true;
        }
        nb_tries++;
    }

    lock->cpu_id = get_cpu()->id;
}

void release(spinlock_t *lock) {
    lock->cpu_id = 0;

    __atomic_thread_fence(__ATOMIC_ACQ_REL);

    __atomic_clear(&lock->held, __ATOMIC_RELEASE);

    pop_off();
}
