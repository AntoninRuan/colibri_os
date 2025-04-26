#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

struct spinlock {
    uint8_t held;

    // Debug value
    uint32_t cpu_id;
};

typedef struct spinlock spinlock_t;

extern spinlock_t spinlocks[512];

void acquire(spinlock_t *);
void release(spinlock_t *);

#endif  // SPINLOCK_H
