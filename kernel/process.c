#include <elf.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/arch/x86-64/tss.h>
#include <kernel/kernel.h>
#include <kernel/list.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/process.h>
#include <kernel/sync.h>
#include <kernel/timer.h>
#include <kernel/x86-64.h>
#include <string.h>

typedef struct lst proc_lst;

proc_lst process_list = {.prev = &process_list, .next = &process_list};
u64 next_free_pid = 0;
spinlock_t free_pid_lock = {.name = "Free pid lock"};
u64 next_free_tid = 0;
spinlock_t free_tid_lock = {.name = "Free tid lock"};

size_t schedule_index;
thread_array schedule_lst;

void init_thread_array(thread_array *array) {
    array->capacity = 8;
    array->len = 0;
    array->data = alloc(array->capacity * sizeof(thread_t *));
}

void add(thread_array *array, thread_t *t) {
    if (array->len >= array->capacity) {
        array->capacity *= 2;
        array->data =
            realloc(array->data, array->capacity * sizeof(thread_t *));
        if (array->data == NULL) return;
    }

    array->data[array->len] = t;
    array->len++;
    return;
}

thread_t *get(thread_array *array, size_t index) { return array->data[index]; }

void remove(thread_array *array, size_t index) {
    if (index == array->len - 1) {
        array->data[index] = NULL;
        array->len--;
        return;
    }

    memmove(&array->data[index], &array->data[index + 1],
            sizeof(thread_t *) * (array->len - index - 1));
    array->len--;
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

    lst_push_end(&process_list, proc);

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

void run_proc(proc_t *p) {
    if (schedule_lst.capacity == 0) init_thread_array(&schedule_lst);
    for (size_t i = 0; i < p->threads.len; i ++) {
        add(&schedule_lst, p->threads.data[i]);
    }
}

void schedule(int_frame_t *context) {
    arm_timer(1e9, false, true);

    if (schedule_lst.capacity == 0) return;

    size_t i = schedule_index;
    thread_t *new_t;
    do {
        new_t = schedule_lst.data[i];
        i ++;
        if (i == schedule_lst.len) i = 0;
    } while (new_t->state != WAITING && i != schedule_index);

    schedule_index = i;
    if (new_t == get_cpu()->thread) return;
    if (new_t->state != WAITING) return; // No thread to run

    u64 rsp = context->registers.rsp;

    thread_t *old_thread = get_cpu()->thread;
    if (old_thread && old_thread->state != DEAD) {
        memcpy(&old_thread->context, context, sizeof(int_frame_t));
        old_thread->state = WAITING;
    } else if (old_thread) {
        destroy_thread(old_thread);
    } else {
        update_rsp0(context->iret_rsp);
    }

    new_t->state = ACTIVE;
    get_cpu()->thread = new_t;
    memcpy(context, &new_t->context, sizeof(int_frame_t));
    change_current_vmm(new_t->proc->vmm);

    context->registers.rsp = rsp;
    return;
}
