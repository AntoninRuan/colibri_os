#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <sys/cdefs.h>

typedef struct {
    void *t;
    u32 exec_start_time;
    u64 vruntime;
} sched_entity_t;

static inline u64 entity_key(sched_entity_t *se) { return se->vruntime; }

#endif  // SCHEDULER_H
