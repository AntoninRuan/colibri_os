#include <elf.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/process.h>
#include <kernel/sync.h>
#include <kernel/x86-64.h>
#include <string.h>

u64 next_free_pid = 0;
spinlock_t free_pid_lock = {.name = "Free pid lock"};

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

    vmm_info_t *vmm = vmm_create(LOWHALF_START, LOWHALF_END, user_proc);
    if (!vmm) goto free_proc;

    map_higher_half(vmm->root_pagetable);
    change_pagetable(vmm->root_pagetable - PHYSICAL_OFFSET);

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
            if (!area || area->start != seg->p_vaddr) goto free_vmm;
            memcpy((void *)area->start, ((void *)elf) + seg->p_offset,
                   seg->p_filesz);
            update_area_access(area, mem_flag);
        }
    }
    memory_area_t *area = vmm_alloc(proc->vmm, PAGE_SIZE, MEMORY_FLAG_WRITE);
    if (!area) goto free_vmm;

    memset((void *)area->start, 0, area->size);
    proc->stack = area;
    proc->context.iret_rsp = area->start + area->size;
    proc->context.iret_rip = elf->e_entry;

    get_cpu()->proc = current;

    if (current)
        change_pagetable(current->vmm->root_pagetable - PHYSICAL_OFFSET);
    else
        change_pagetable(kernel_vmm.root_pagetable - PHYSICAL_OFFSET);

    return proc;

free_vmm:
    get_cpu()->proc = current;
    vmm_destroy(vmm);

free_proc:
    free(proc);

    return NULL;
}
