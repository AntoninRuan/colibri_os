#ifndef PROCESS_H
#define PROCESS_H

#ifdef __arch_x86_64
#include <kernel/x86-64.h>
#endif
#include <elf.h>
#include <kernel/list.h>
#include <kernel/memory/vmm.h>
#include <stdint.h>

#define PROC_NAME_MAX_LEN 256

enum process_state { ACTIVE, SLEEPING, WAITING, DEAD };

typedef enum process_state process_state_t;

typedef struct process proc_t;
struct process {
    struct lst _;

    u64 id;
    char name[PROC_NAME_MAX_LEN];
    process_state_t state;
    int_frame_t context;

    vmm_info_t *vmm;
    memory_area_t *stack;
    memory_area_t *heap_start;
};

proc_t *create_process(char *name, Elf64_Ehdr *elf, bool user_proc);

#endif  // PROCESS_H
