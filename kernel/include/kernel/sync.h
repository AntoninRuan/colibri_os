#ifndef SYNC_H
#define SYNC_H

#include <sys/cdefs.h>

struct spinlock {
    u8 held;

    // Debug value
    u32 cpu_id;
    char *name;
};

typedef struct spinlock spinlock_t;

void acquire(spinlock_t *);
void release(spinlock_t *);

#endif  // SYNC_H
