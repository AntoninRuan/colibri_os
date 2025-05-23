#ifndef PROCESS_H
#define PROCESS_H

#ifdef __arch_x86_64
#include <kernel/x86-64.h>
#endif
#include <kernel/list.h>
#include <kernel/memory/vmm.h>
#include <stdint.h>

enum process_state { ACTIVE, SLEEPING, WAITING, DEAD };

typedef enum process_state process_state_t;

typedef struct process proc_t;
struct process {
    struct lst _;

    u64 id;
    process_state_t state;
    int_frame_t context;

    vmm_info_t *vmm;
    memory_area_t *stack;
    memory_area_t *heap_start;
};

#endif  // PROCESS_H
