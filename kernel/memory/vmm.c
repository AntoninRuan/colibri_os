#include <kernel/log.h>
#include <kernel/memory/memory_layout.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/sync.h>
#include <stdint.h>
#include <string.h>

vmm_info_t kernel_vmm = { .lock.name = "Kernel vmm" };
vmm_info_t *current_vmm = &kernel_vmm;

void vmm_init(vmm_info_t *vmm, void *pagetable, uintptr_t start, uintptr_t end,
              bool user) {
    memset(vmm, 0, sizeof(vmm_info_t));
    vmm->root_pagetable = pagetable;
    vmm->user_vmm = user;

    vmm->vmm_start = start + PAGE_SIZE;
    vmm->vmm_data_start = vmm->vmm_start;
    vmm->vmm_data_end = end;
}

memory_area_t *get_memory_area(vmm_info_t *vmm, void *va) {
    if ((uintptr_t)va > vmm->current_addr) return NULL;

    memory_area_t *cur = vmm->first_area;
    for (; cur; cur = cur->next) {
        if (cur->start <= (uintptr_t)va &&
            (uintptr_t)va < cur->start + cur->size) {
            return cur;
        }
    }
    return NULL;
}

memory_area_t *vmm_alloc(vmm_info_t *vmm, uint64_t sz, uint8_t flags) {
    return vmm_alloc_at(0, vmm, sz, flags);
}

memory_area_t *vmm_alloc_at(uintptr_t base, vmm_info_t *vmm, uint64_t sz,
                            uint8_t flags) {
    uint64_t length = PAGE_END(sz - 1, SMALL_PAGE_SIZE);
    uintptr_t low_bound;
    uintptr_t high_bound = vmm->vmm_data_end;
    uintptr_t found = 0;

    acquire(&vmm->lock);
    memory_area_t *cur = vmm->current_area;
    memory_area_t *prev = NULL;
    if (cur == NULL)
        low_bound = vmm->vmm_data_start;
    else
        low_bound = PAGE_END(vmm->current_addr, PAGE_SIZE) + 1;

    found = low_bound;
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
        release(&vmm->lock);
        return NULL;
    }

    // Current container is full creating a new one, or there is no existing
    // container
    if (vmm->current_index == ITEM_PER_CONTAINER ||
        vmm->root_container == NULL) {
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
    release(&vmm->lock);
    return new;
}

int vmm_free(vmm_info_t *vmm, memory_area_t *area) {
    // Check if area is managed by vmm
    vmm_container_t *cur = vmm->root_container;
    while (cur) {
        if (cur->root_area <= area &&
            area <= &cur->root_area[ITEM_PER_CONTAINER - 1]) {
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

    unmappages(vmm->root_pagetable, (void *)area->start, area->size, true);

    // TODO add mechanism to free / compact containers

    return 0;
}

int on_demand_allocation(void *va) {
    void *page = kalloc();
    memory_area_t *area = get_memory_area(current_vmm, va);
    if (area == NULL) {
        // va is not an allocated address for current vmm
        return -1;
    }
    mappages(current_vmm->root_pagetable, va, PAGE_SIZE, page, area->flags);
    return 0;
}
