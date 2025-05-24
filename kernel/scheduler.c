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

typedef struct lst proc_lst;

bool scheduler_ready = false;
proc_lst schedule_lst;
spinlock_t proc_list_lock = {.name = "Proc list lock"};

void init_scheduler() {
    lst_init(&schedule_lst);
    scheduler_ready = true;
}

void run_proc(proc_t *p) {
    lst_push_end(&schedule_lst, p);
}

void schedule(int_frame_t *int_frame) {
    if (!scheduler_ready) return;
    if (lst_empty(&schedule_lst)) return;

    acquire(&proc_list_lock);
    proc_t *new_proc = lst_pop_end(&schedule_lst);
    release(&proc_list_lock);

    if (!new_proc) return;

    u64 rsp = int_frame->registers.rsp;

    proc_t *old_proc = get_cpu()->proc;
    if (old_proc && old_proc->state != DEAD) {
        memcpy(&old_proc->context, int_frame, sizeof(int_frame_t));
        lst_push_end(&schedule_lst, old_proc);
    } else if (old_proc) {
        // destroy_process(old_proc);
    } else {
        update_rsp0(int_frame->iret_rsp);
    }

    get_cpu()->proc = new_proc;
    memcpy(int_frame, &new_proc->context, sizeof(int_frame_t));
    change_pagetable(new_proc->vmm->root_pagetable - PHYSICAL_OFFSET);

    int_frame->registers.rsp = rsp;
    return;
}
