#include <elf.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/arch/x86-64/tss.h>
#include <kernel/kernel.h>
#include <kernel/list.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/memory_layout.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/process.h>
#include <kernel/scheduler.h>
#include <kernel/sync.h>
#include <kernel/x86-64.h>
#include <stdbool.h>
#include <string.h>

typedef struct lst thread_lst;

thread_lst schedule_lst;
spinlock_t schedule_lock = {.name = "Schedule  list lock"};
bool scheduler_ready = false;

void init_scheduler() {
    lst_init(&schedule_lst);
    scheduler_ready = true;
}

void run_proc(proc_t *p) {
    for (size_t i = 0; i < p->threads.len; i ++) {
        lst_push_end(&schedule_lst, p->threads.data[i]);
    }
}

void schedule(int_frame_t *int_frame) {
    if (!scheduler_ready) return;
    if (lst_empty(&schedule_lst)) return;

    acquire(&schedule_lock);
    thread_t *new_thread = lst_pop(&schedule_lst);
    release(&schedule_lock);

    if (!new_thread) return;

    u64 rsp = int_frame->registers.rsp;

    thread_t *old_thread = get_cpu()->thread;
    if (old_thread && old_thread->state != DEAD) {
        memcpy(&old_thread->context, int_frame, sizeof(int_frame_t));
        lst_push_end(&schedule_lst, old_thread);
    } else if (old_thread) {
        destroy_thread(old_thread);
    } else {
        update_rsp0(int_frame->iret_rsp);
    }

    get_cpu()->thread = new_thread;
    memcpy(int_frame, &new_thread->context, sizeof(int_frame_t));
    change_current_vmm(new_thread->proc->vmm);

    int_frame->registers.rsp = rsp;
    return;
}
