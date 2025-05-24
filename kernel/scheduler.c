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

u64 next_free_pid = 0;
spinlock_t free_pid_lock = {.name = "Free pid lock"};

bool scheduler_ready = false;
proc_lst schedule_lst;
spinlock_t proc_list_lock = {.name = "Proc list lock"};

void init_scheduler() {
    lst_init(&schedule_lst);
    scheduler_ready = true;
}

proc_t *create_process(char *name, Elf64_Ehdr *elf, bool user_proc) {
    proc_t *current = get_cpu()->proc;
    proc_t *proc = alloc(NULL, sizeof(proc_t));
    if (!proc) return NULL;

    acquire(&free_pid_lock);
    proc->id = next_free_pid++;
    release(&free_pid_lock);

    proc->state = ACTIVE;
    memset(&proc->context, 0, sizeof(int_frame_t));
    proc->context.iret_flags = 0x202;
    proc->context.iret_cs = 0x18 | 3;
    proc->context.iret_ss = 0x20 | 3;
    strncpy(proc->name, name, PROC_NAME_MAX_LEN);

    void *pagetable = kalloc() + PHYSICAL_OFFSET;
    if (!pagetable) goto free_proc;

    map_higher_half(pagetable);
    change_pagetable(pagetable - PHYSICAL_OFFSET);

    vmm_info_t *vmm = alloc(NULL, sizeof(vmm_info_t));
    if (!vmm) goto free_pt;

    spinlock_t *vmm_lock = alloc(NULL, sizeof(spinlock_t));
    if (!vmm_lock) goto free_vmm;

    memset(vmm_lock, 0, sizeof(spinlock_t));
    vmm_init(vmm, pagetable, 0, LOWHALF_END, user_proc, vmm_lock);
    proc->vmm = vmm;

    get_cpu()->proc = proc;
    for (u64 i = 0; i < elf->e_phnum; i++) {
        Elf64_Phdr *seg = get_segment(elf, i);
        if (seg->p_type == PT_LOAD) {
            u8 mem_flag = 0;
            if (seg->p_flags & PF_W) mem_flag |= MEMORY_FLAG_WRITE;
            if (seg->p_flags & PF_X) mem_flag |= MEMORY_FLAG_EXEC;
            memory_area_t *area =
                vmm_alloc_at(seg->p_vaddr, proc->vmm, seg->p_memsz,
                             mem_flag | MEMORY_FLAG_WRITE);
            if (!area || area->start != seg->p_vaddr) goto free_vmm_lock;
            memcpy((void *)area->start, ((void *)elf) + seg->p_offset,
                   seg->p_filesz);
            update_area_access(area, mem_flag);
        }
    }
    memory_area_t *area = vmm_alloc(proc->vmm, PAGE_SIZE, MEMORY_FLAG_WRITE);
    if (!area) goto free_vmm_lock;

    memset((void *)area->start, 0, area->size);
    proc->stack = area;
    proc->context.iret_rsp = area->start + area->size;
    proc->context.iret_rip = elf->e_entry;

    get_cpu()->proc = current;
    lst_push_end(&schedule_lst, proc);

    if (current)
        change_pagetable(current->vmm->root_pagetable - PHYSICAL_OFFSET);

    return proc;

free_vmm_lock:
    free(vmm_lock);
    get_cpu()->proc = current;

free_vmm:
    clear_vmm(proc->vmm);
    free(vmm);

free_pt:
    change_pagetable(current->vmm->root_pagetable - PHYSICAL_OFFSET);
    kfree(pagetable - PHYSICAL_OFFSET);

free_proc:
    free(proc);

    return NULL;
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
