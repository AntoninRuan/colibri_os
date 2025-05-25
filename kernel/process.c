#include <elf.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/list.h>
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
u64 next_free_tid = 0;
spinlock_t free_tid_lock = {.name = "Free tid lock"};

void init_thread_array(thread_arr_t *array) {
    array->capacity = 8;
    array->len = 0;
    array->data = alloc(array->capacity * sizeof(thread_t *));
}

void add(thread_arr_t *array, thread_t *t) {
    if (array->len >= array->capacity) {
        array->capacity *= 2;
        array->data =
            realloc(array->data, array->capacity * sizeof(thread_t *));
    }

    array->data[array->len] = t;
    array->len++;
    return;
}

thread_t *get(thread_arr_t *array, size_t index) { return array->data[index]; }

void remove(thread_arr_t *array, size_t index) {
    if (index == array->len - 1) {
        array->data[index] = NULL;
        array->len--;
        return;
    }

    memmove(&array->data[index], &array->data[index + 1],
            sizeof(thread_t *) * (array->len - index - 1));
    array->len --;
    return;
}

proc_t *create_process(char *name, Elf64_Ehdr *elf, bool user_proc) {
    vmm_info_t *current_vmm = get_cpu()->vmm;
    proc_t *proc = alloc(sizeof(proc_t));
    if (!proc) return NULL;

    acquire(&free_pid_lock);
    proc->id = next_free_pid++;
    release(&free_pid_lock);

    strncpy(proc->name, name, PROC_NAME_LEN);

    vmm_info_t *vmm = vmm_create(LOWHALF_START, LOWHALF_END, user_proc);
    if (!vmm) goto free_proc;

    map_higher_half(vmm->root_pagetable);
    change_current_vmm(vmm);

    proc->vmm = vmm;
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
            update_area_access(vmm, area, mem_flag);
        }
    }

    if (current_vmm) change_current_vmm(current_vmm);

    init_thread_array(&proc->threads);
    thread_t *t = add_thread(proc, "", elf->e_entry);
    if (!t) goto free_vmm;

    return proc;

free_vmm:
    change_current_vmm(current_vmm);
    vmm_destroy(vmm);

free_proc:
    free(proc);

    return NULL;
}

void destroy_process(proc_t *proc) {
    vmm_destroy(proc->vmm);
    free(proc);
}

thread_t *add_thread(proc_t *proc, char *name, u64 entry) {
    thread_t *t = alloc(sizeof(thread_t));
    if (!t) return NULL;

    acquire(&free_tid_lock);
    t->tid = next_free_tid++;
    release(&free_tid_lock);

    t->state = WAITING;
    memset(&t->context, 0, sizeof(int_frame_t));
    t->context.iret_flags = 0x202;
    t->context.iret_cs = 0x18 | 3;
    t->context.iret_ss = 0x20 | 3;

    strncpy(t->name, name, THREAD_NAME_LEN);

    memory_area_t *stack = vmm_alloc(proc->vmm, PAGE_SIZE, MEMORY_FLAG_WRITE);
    if (!stack) {
        free(t);
        return NULL;
    }

    // memset((void *)stack->start, 0, stack->size);
    t->stack = stack;
    t->context.iret_rsp = stack->start + stack->size;
    t->context.iret_rip = entry;

    t->proc = proc;

    add(&proc->threads, t);

    return t;
}

void destroy_thread(thread_t *t) {}
