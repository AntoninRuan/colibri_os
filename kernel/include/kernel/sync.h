#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>

struct spinlock {
    uint8_t held;

    // Debug value
    uint32_t cpu_id;
    char *name;
};

typedef struct spinlock spinlock_t;

void acquire(spinlock_t *);
void release(spinlock_t *);

#endif  // SYNC_H
