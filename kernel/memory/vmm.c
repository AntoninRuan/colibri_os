#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/memory_layout.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/sync.h>
#include <kernel/x86-64.h>
#include <stdint.h>
#include <string.h>

spinlock_t kernel_vmm_lock = {.name = "Kernel vmm"};
vmm_info_t kernel_vmm = {0};

void vmm_init(vmm_info_t *vmm, void *pagetable, uintptr_t start, uintptr_t end,
              bool user, spinlock_t *lock) {
    memset(vmm, 0, sizeof(vmm_info_t));
    vmm->root_pagetable = pagetable;
    vmm->user_vmm = user;

    vmm->vmm_start = PAGE_START(start, PAGE_SIZE);
    if (vmm->vmm_start == 0) vmm->vmm_start += PAGE_SIZE;
    vmm->vmm_data_start = vmm->vmm_start;
    vmm->vmm_data_end = PAGE_END(end, PAGE_SIZE) + 1;

    if (vmm != &kernel_vmm)
        vmm->lock = lock;
    else
        vmm->lock = &kernel_vmm_lock;
}

// return memory area containing va in vmm, return NULL if no area contains va
memory_area_t *get_memory_area(vmm_info_t *vmm, void *va) {
    if (vmm == NULL) return NULL;
    if ((uintptr_t)va > vmm->current_addr) return NULL;

    memory_area_t *cur = vmm->first_area;
    for (; cur; cur = cur->next) {
        if (cur->start <= (uintptr_t)va
            && (uintptr_t)va < cur->start + cur->size) {
            return cur;
        }
    }
    return NULL;
}

memory_area_t *vmm_alloc(vmm_info_t *vmm, u64 sz, u8 flags) {
    return vmm_alloc_at(0, vmm, sz, flags);
}

memory_area_t *vmm_alloc_at(uintptr_t base, vmm_info_t *vmm, u64 sz, u8 flags) {
    u64 length = PAGE_END(sz - 1, SMALL_PAGE_SIZE);
    uintptr_t low_bound;
    uintptr_t high_bound = vmm->vmm_data_end;
    uintptr_t found = 0;

    acquire(vmm->lock);
    memory_area_t *cur = vmm->current_area;
    memory_area_t *prev = NULL;
    if (cur == NULL)
        low_bound = vmm->vmm_data_start;
    else
        low_bound = PAGE_END(vmm->current_addr, PAGE_SIZE) + 1;

    found = low_bound;
    base = PAGE_START(base, PAGE_SIZE);
    if (found < base) found = base;
    if (found + length > high_bound) {
        // In case the last given address is too high, search for memory avaible
        // in gaps
        cur = vmm->first_area;
        while (cur) {
            if (cur->start + cur->size - 1 < base) {
                prev = cur;
                cur = cur->next;
                continue;
            }
            if (prev == NULL)
                low_bound = vmm->vmm_data_start;
            else
                low_bound =
                    PAGE_END(prev->start + prev->size - 1, SMALL_PAGE_SIZE) + 1;

            if (cur == NULL)
                high_bound = vmm->vmm_data_end;
            else
                high_bound = cur->start;

            if (low_bound + length < high_bound) {
                found = low_bound;
                break;
            }

            prev = cur;
            cur = cur->next;
        }
        found = 0;
    } else {
        // We allocate an area after current
        // Update variable to have the following order in the list
        // prev -> new -> cur
        prev = cur;
        cur = NULL;
    }

    if (!found) {
        release(vmm->lock);
        return NULL;
    }

    // Current container is full creating a new one, or there is no existing
    // container
    if (vmm->current_index == ITEM_PER_CONTAINER
        || vmm->root_container == NULL) {
        void *phy_addr = kalloc();
        vmm_container_t *new_container =
            (vmm_container_t *)(phy_addr + PHYSICAL_OFFSET);

        if (vmm->current_container) {
            // There is already a container created
            vmm->current_container->next = new_container;
        } else {
            // There is no existing container, setting the new one
            // as root_container
            vmm->root_container = new_container;
        }

        vmm->current_container = new_container;
        vmm->current_index = 0;
    }

    memory_area_t *new = &vmm->current_container->root_area[vmm->current_index];
    vmm->current_index++;
    new->start = found;
    new->size = length + 1;
    new->flags = flags;
    if (vmm->user_vmm) new->flags |= MEMORY_FLAG_USER;

    new->next = cur;
    if (cur) cur->prev = new;

    new->prev = prev;
    if (prev)
        prev->next = new;
    else {
        vmm->first_area = new;
    }
    if (prev == vmm->current_area) {
        vmm->current_area = new;
        vmm->current_addr = new->start + new->size - 1;
    }
    release(vmm->lock);
    return new;
}

int vmm_free_area(vmm_info_t *vmm, memory_area_t *area) {
    // Check area is not null
    if (!area) return -1;

    // Check if area is managed by vmm
    vmm_container_t *cur = vmm->root_container;
    while (cur) {
        if (cur->root_area <= area
            && area <= &cur->root_area[ITEM_PER_CONTAINER - 1]) {
            break;
        }
        cur = cur->next;
    }

    // area is not in any container of vmm
    if (!cur) return -1;

    if (area->prev)
        area->prev->next = area->next;
    else
        // area was the first area
        // setting the next as the first one
        vmm->first_area = area->next;

    if (area->next)
        area->next->prev = area->prev;
    else
        vmm->current_area = area->prev;

    unmappages(vmm->root_pagetable, (void *)area->start, area->size,
               !area->is_mmio);

    // TODO add mechanism to free / compact containers

    return 0;
}

void change_current_vmm(vmm_info_t *vmm) {
    get_cpu()->vmm = vmm;
    if (vmm != &kernel_vmm) {
        change_pagetable(vmm->root_pagetable - PHYSICAL_OFFSET);
    }
}

vmm_info_t *get_current_vmm() {
    if (get_cpu()->vmm)
        return get_cpu()->vmm;
    else
        return &kernel_vmm;
}

vmm_info_t *get_relevant_vmm(void *va) {
    if ((u64)va >= 0xFFFF800000000000)
        return &kernel_vmm;
    else
        return get_current_vmm();
}

int vmm_free(void *va) {
    vmm_info_t *vmm = get_relevant_vmm(va);

    memory_area_t *area = get_memory_area(vmm, va);
    if (!area) return -1;

    return vmm_free_area(vmm, area);
}

int update_area_access(vmm_info_t *vmm, memory_area_t *area, u8 flags) {
    area->flags = flags;
    if (vmm->user_vmm) area->flags |= MEMORY_FLAG_USER;

    return updatepages(vmm->root_pagetable, (void *)area->start, area->size,
                       area->flags);
}

int on_demand_allocation(void *va) {
    vmm_info_t *current_vmm = get_relevant_vmm(va);

    memory_area_t *area = get_memory_area(current_vmm, va);
    if (area == NULL) {
        // va is not an allocated address for current vmm
        return -1;
    }

    void *page = kalloc();
    if (mappages(current_vmm->root_pagetable, (void *)PAGE_START(va, PAGE_SIZE),
                 PAGE_SIZE, page, area->flags)) {
        return -1;
    }
    return 0;
}

// Create a vmm, need to have kernel heap setup
vmm_info_t *vmm_create(uintptr_t start, uintptr_t end, bool user) {
    vmm_info_t *vmm = alloc(sizeof(vmm_info_t));
    if (!vmm) return NULL;

    void *pagetable = kalloc();
    if (!pagetable) goto free_vmm;
    pagetable += PHYSICAL_OFFSET;

    spinlock_t *vmm_lock = alloc(sizeof(spinlock_t));
    if (!vmm_lock) goto free_pt;
    memset(vmm_lock, 0, sizeof(spinlock_t));

    vmm_init(vmm, pagetable, start, end, user, vmm_lock);

    return vmm;

free_pt:
    kfree(pagetable - PHYSICAL_OFFSET);

free_vmm:
    free(vmm);

    return NULL;
}

// Remove all allocated memory in vmm
void vmm_destroy(vmm_info_t *vmm) {
    memory_area_t *cur_area = vmm->first_area;
    while (cur_area) {
        unmappages(vmm->root_pagetable, (void *)cur_area->start, cur_area->size,
                   true);

        cur_area = cur_area->next;
    }

    vmm_container_t *cur_container = vmm->root_container;
    while (cur_container) {
        vmm_container_t *old = cur_container;
        cur_container = cur_container->next;

        kfree((void *)old - PHYSICAL_OFFSET);
    }

    free(vmm->lock);
    kfree(vmm->root_pagetable - PHYSICAL_OFFSET);
    free(vmm);
}

void *map_mmio(vmm_info_t *vmm, u64 physical, size_t size, bool writable) {
    if (vmm == NULL) vmm = &kernel_vmm;

    u8 flag = 0;
    if (writable) flag |= MEMORY_FLAG_WRITE;
    if (vmm->user_vmm) flag |= MEMORY_FLAG_USER;
    memory_area_t *area = vmm_alloc(vmm, size, flag);
    if (!area) return NULL;
    area->is_mmio = true;
    int result = mappages(vmm->root_pagetable, (void *)area->start, area->size,
                          (void *)physical, area->flags);
    if (result) {
        logf(ERROR, "Error while mapping IO");
        return NULL;
    };

    return (void *)area->start;
}

void unmap_mmio(void *va) {
    vmm_info_t *vmm = get_relevant_vmm(va);
    memory_area_t *area = get_memory_area(vmm, va);

    // va is not allocated
    if (!area) return;

    // area is not mmio, illegal call
    if (!area->is_mmio) return;

    vmm_free_area(vmm, area);
}
