#ifndef CFS_H
#define CFS_H

#include <kernel/scheduler.h>
#include <kernel/process.h>

sched_entity_t *cfs_pick_next();
void cfs_queue(sched_entity_t *);
void cfs_dequeue(sched_entity_t *);
u64 get_timeslice(thread_t *t);

#endif // CFS_H
