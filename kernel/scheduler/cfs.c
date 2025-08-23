#include <kernel/kernel.h>
#include <kernel/scheduler.h>
#include <kernel/scheduler/cfs.h>
#include <kernel/scheduler/rb_tree.h>
#include <math.h>
#include <sys/cdefs.h>

typedef struct {
    sched_tree_t tree;
    u64 running;
} cfs_t;

cfs_t cfs = {0};

u64 weight_delta(sched_entity_t *se, u64 delta) {
    u8 prio = ((thread_t *)se)->prio;
    return delta * (1 + prio);
}

void update(sched_entity_t *se) {
    u64 now = uptime();
    u64 delta = now - se->exec_start_time;

    u64 delta_w = weight_delta(se, delta);
    se->vruntime += delta_w;
}

void cfs_queue(sched_entity_t *se) {
    update(se);
    cfs.running++;
    rb_insert(&cfs.tree, se);
}

void cfs_dequeue(sched_entity_t *se) {
    cfs.running--;
    rb_remove(&cfs.tree, se);
}

#define TIME_UNIT   100e6  // in nanosec
#define GRANULARITY 1e6

u64 get_timeslice(thread_t *t) {
    return max(TIME_UNIT / ((1 + t->prio) * (cfs.running + 1)), GRANULARITY);
}

sched_entity_t *cfs_pick_next() {
    if (cfs.tree.leftmost) return cfs.tree.leftmost->se;
    return NULL;
}
