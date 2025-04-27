#ifndef VMM_H
#define VMM_H

#include <kernel/memory/memory_layout.h>
#include <kernel/sync.h>
#include <stddef.h>
#include <stdint.h>

struct memory_area;
typedef struct memory_area memory_area_t;
struct memory_area {
    uint64_t start;
    uint64_t size;
    uint8_t flags;
    memory_area_t *next;
    memory_area_t *prev;
};

#define MEMORY_FLAG_WRITE 1
#define MEMORY_FLAG_USER  (1 << 1)
#define MEMORY_FLAG_EXEC  (1 << 2)

#define ITEM_PER_CONTAINER (PAGE_SIZE / sizeof(memory_area_t))
struct vmm_container {
    memory_area_t root_area[ITEM_PER_CONTAINER];
    struct vmm_container *next;
};

typedef struct vmm_container vmm_container_t;

struct vmm_info {
    void *root_pagetable;
    bool user_vmm;

    uintptr_t vmm_start;
    uintptr_t vmm_data_start;
    uintptr_t vmm_data_end;

    size_t current_index;

    vmm_container_t *root_container;
    vmm_container_t *current_container;

    uintptr_t current_addr;
    memory_area_t *first_area;
    memory_area_t *current_area;

    spinlock_t *lock;
};

typedef struct vmm_info vmm_info_t;

extern vmm_info_t kernel_vmm;
extern vmm_info_t *current_vmm;

void vmm_init(vmm_info_t *vmm, void *pagetable, uintptr_t start, uintptr_t end,
              bool user, spinlock_t *spinlock);
memory_area_t *get_memory_area(vmm_info_t *vmm, void *va);
memory_area_t *vmm_alloc_at(uintptr_t base, vmm_info_t *, uint64_t sz,
                            uint8_t flags);
memory_area_t *vmm_alloc(vmm_info_t *vmm, uint64_t sz, uint8_t flags);
int vmm_free(vmm_info_t *vmm, memory_area_t *area);
int on_demand_allocation(void *va);

#endif  // VMM_H
