#ifndef PROCESS_H
#define PROCESS_H

#ifdef __arch_x86_64
#include <kernel/x86-64.h>
#endif  // __arch_x86_64

#include <elf.h>
#include <kernel/list.h>
#include <kernel/memory/vmm.h>
#include <sys/cdefs.h>

#define PROC_NAME_LEN   64
#define THREAD_NAME_LEN 64

enum thread_state { ACTIVE, SLEEPING, WAITING, DEAD };

typedef enum thread_state thread_state_t;
typedef struct thread thread_t;
typedef struct process proc_t;
typedef struct lst thread_lst;
typedef struct thread_arr thread_array;

struct thread_arr {
    thread_t **data;
    size_t len;
    size_t capacity;
};

struct process {
    struct lst _;

    u64 id;
    char name[PROC_NAME_LEN];

    thread_array threads;

    vmm_info_t *vmm;
    memory_area_t *heap_start;
};

struct thread {
    u64 tid;
    char name[THREAD_NAME_LEN];

    thread_state_t state;
    proc_t *proc;

    int_frame_t context;
    memory_area_t *stack;
};

proc_t *create_process(char *name, Elf64_Ehdr *elf, bool user_proc);
void destroy_process(proc_t *proc);

void run_proc(proc_t *);

thread_t *add_thread(proc_t *proc, char *name, u64 entry);
void destroy_thread(thread_t *t);

void schedule(int_frame_t *);

#endif  // PROCESS_H
