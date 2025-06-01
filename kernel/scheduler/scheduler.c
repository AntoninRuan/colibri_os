#include <kernel/arch/x86-64/tss.h>
#include <kernel/kernel.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vmm.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>
#include <kernel/scheduler/cfs.h>
#include <kernel/scheduler/rb_tree.h>
#include <kernel/timer.h>
#include <kernel/x86-64.h>
#include <string.h>
#include <sys/cdefs.h>

thread_t idle_t = {
    .name = "Idle thread",
    .context.iret_rip = (u64) &idle
};

void schedule(int_frame_t *context) {
    u64 rsp = context->registers.rsp;

    thread_t *old_thread = get_cpu()->thread;

    sched_entity_t *se = cfs_pick_next();
    thread_t *new_thread = (thread_t *)se;

    if (!old_thread || old_thread == &idle_t) {
        update_rsp0(context->iret_rsp);
    } else if (old_thread->state != DEAD) {
        if (!new_thread) {
            // No other thread to run old can continue
            new_thread = old_thread;
        } else {
            old_thread->state = WAITING;
            cfs_queue(&old_thread->se);
            memcpy(&old_thread->context, context, sizeof(int_frame_t));
        }
    } else {
        destroy_thread(old_thread);
    }

    if (new_thread == NULL) {
        get_cpu()->thread = &idle_t;
        context->iret_rip = idle_t.context.iret_rip;
        context->iret_rsp = get_rsp0();
        change_current_vmm(&kernel_vmm);
        arm_timer(1e6, false, true);
        return;
    }

    if (new_thread != old_thread) {
        cfs_dequeue(se);
        new_thread->state = ACTIVE;
        get_cpu()->thread = new_thread;
        memcpy(context, &new_thread->context, sizeof(int_frame_t));
        change_current_vmm(new_thread->proc->vmm);
    }
    u64 timeslice = get_timeslice(new_thread);

    arm_timer(timeslice, false, true);
    context->registers.rsp = rsp;
    return;
}
