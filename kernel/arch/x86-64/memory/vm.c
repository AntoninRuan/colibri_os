#include <cpuid.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <kernel/multiboot2.h>
#include <kernel/x86-64.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

alignas(4096) pdpte_t physical_mapping[512] = {0};  // Goes to pml4[509]

bool big_page_size_supported;
extern u8 pml4;
pml4e_t *kernel_pml4;

void enable_nx_flag() {
    u64 ia32_efer = rdmsr(IA32_EFER);
    ia32_efer |= (1L << 11);
    wrmsr(IA32_EFER, ia32_efer);
}

void disable_id_mapping() {
    pde_t *kernel_p2 = (pde_t *)pd_va(0, 0);
    kernel_p2->present = false;
}

void kvminit(struct multiboot_memory_map *mmap) {
    log(INFO, "Init virtual and physical memory");
    kernel_pml4 = (pml4e_t *)&pml4;

    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    kernel_status.nx_flag_enabled = (edx & (1L << 20)) != 0;
    big_page_size_supported = (edx & (1L << 26)) != 0;

    if (kernel_status.nx_flag_enabled) enable_nx_flag();

    pml4e_t physical_entry = {0};
    physical_entry.raw = (u64)(PHYSICAL_ADDRESS(physical_mapping));
    physical_entry.present = true;
    physical_entry.writable = true;
    physical_entry.xd = kernel_status.nx_flag_enabled;
    ((pml4e_t *)(pdpt_va(PML4_RECURSE_ENTRY)))[509] = physical_entry;

    // Determine RAM area
    u64 max_ram_base_addr = 0;
    u64 max_ram_size = 0;
    struct multiboot_mmap_entry entry;
    u32 entry_number = (mmap->size - 16) / mmap->entry_size;
    for (u32 i = 0; i < entry_number; i++) {
        entry = mmap->entries[i];
        if (entry.type == 1 && entry.length > max_ram_size) {
            max_ram_size = entry.length;
            max_ram_base_addr = entry.base_addr;
        }
    }

    if (!max_ram_size) panic("kvminit: no ram detected");

    u64 phys_memory_end = max_ram_base_addr + max_ram_size - 1;

    u64 kernel_physical_end = PHYSICAL_ADDRESS(_kernel_virtual_end);
    memory_area_t ram_available = {0};
    ram_available.start = PAGE_END(kernel_physical_end, SMALL_PAGE_SIZE) + 1;
    ram_available.size = phys_memory_end - ram_available.start + 1;

    // Map ram area for easy access in allocators
    for (int i = 0; i < 512; i++) {
        pdpte_t entry = {0};
        entry.raw = (u64)BIG_PAGE_SIZE * i;
        entry.present = true;
        entry.writable = true;
        entry.xd = kernel_status.nx_flag_enabled;
        entry.page_size = 1;
        physical_mapping[i] = entry;
    }

    init_phys_allocator(&ram_available);
    vmm_init(&kernel_vmm, kernel_pml4, 0xFFFF800000000000, 0xFFFFFE8000000000,
             false, NULL);
}

u64 vmflag_to_x86flag(u64 flag) {
    u64 result = 0;
    if (kernel_status.nx_flag_enabled && !(flag & MEMORY_FLAG_EXEC)) {
        result |= 1L << 63;
    }
    result |= (flag & (MEMORY_FLAG_WRITE | MEMORY_FLAG_USER)) << 1;
    return result;
}

// Return physical-mapped address of the page descriptor for virtual
// address va in pagetable, if alloc is true create the necessary
// intermediate, else if one is missing return null
void *walk(void *pagetable, void *va, bool alloc) {
    u64 addr = (u64)va;
    pdpte_t *current_pt = (pdpte_t *)pagetable;
    pdpte_t *entry;
    u64 index;

    for (int level = 3; level > 0; level--) {
        index = VA2INDEX(addr, level);
        entry = &current_pt[index];
        if (entry->present) {
            if (entry->page_size) {
                return (void *)entry;
            }
            current_pt = (void *)((entry->pd_addr << 12) + PHYSICAL_OFFSET);
        } else {
            void *new_pt;
            if (!alloc || (new_pt = kalloc()) == 0) return 0;
            entry->raw = (u64)new_pt;
            entry->present = true;
            entry->writable = true;
            entry->user_page = true;
            current_pt = (void *)(new_pt + PHYSICAL_OFFSET);
            memset(current_pt, 0, PAGE_SIZE);
        }
    }
    return &current_pt[VA2INDEX(addr, 0)];
}

// Recursively free a pagetable
void freewalk(void *pagetable, u8 level) {
    pdpte_t *table = (pdpte_t *)pagetable;

    for (int i = 0; i < 512; i++) {
        pdpte_t entry = table[i];
        if (!entry.present) continue;

        if (!entry.page_size && level) {
            // We found a child, freeing it
            void *child = (void *)(entry.pd_addr << 12);
            freewalk(child + PHYSICAL_OFFSET, level - 1);
            entry.present = false;
            kfree(child);
        } else if (entry.page_size || !level) {
            // leaf
            panic("freewalk: some leaf were not freed");
        }
    }
}

// Map virtual address va in pagetable to pa up to va + sz
// Address are rounded to be page aligned
// Return 0 on success, -1 on error
int mappages(void *pagetable, void *va, u64 sz, void *pa, u8 flags) {
    void *current = (void *)PAGE_START(va, PAGE_SIZE);
    void *current_pa = (void *)PAGE_START(pa, PAGE_SIZE);
    void *end = (void *)PAGE_END(va + sz - 1, PAGE_SIZE);
    u64 x86_flags = vmflag_to_x86flag(flags);

    // TODO detection in case (va + sz) overflows or is a non canonical address
    while (current < end) {
        pte_t *descriptor = (pte_t *)walk(pagetable, current, true);
        if (descriptor == 0 || descriptor->present) {
            unmappages(pagetable, va, current - va, false);
            return -1;
        }

        descriptor->raw = (u64)current_pa;
        descriptor->raw |= x86_flags;
        descriptor->present = true;
        current += PAGE_SIZE;
        current_pa += PAGE_SIZE;
    }

    return 0;
}

int updatepages(void *pagetable, void *va, u64 sz, u8 flags) {
    void *current = (void *)PAGE_START(va, SMALL_PAGE_SIZE);
    void *end = (void *)PAGE_END(va + sz - 1, SMALL_PAGE_SIZE);
    u64 x86_flags = vmflag_to_x86flag(flags);

    for (; current < end; current += PAGE_SIZE) {
        pte_t *descriptor = walk(pagetable, current, false);
        if (descriptor == 0) return 1;

        descriptor->raw &= ~PAGE_FLAG_MASK;
        descriptor->raw |= x86_flags;
    }

    return 0;
}

// Unmap all pages starting from va up to PAGE_END(va+sz)
// Eventually free the physical page if free is true
// Return 0 on success, -1 on error
int unmappages(void *pagetable, void *va, u64 sz, bool free) {
    void *current = (void *)PAGE_START(va, SMALL_PAGE_SIZE);
    void *end = (void *)PAGE_END(va + sz - 1, SMALL_PAGE_SIZE);

    for (; current < end; current += PAGE_SIZE) {
        pte_t *descriptor = walk(pagetable, current, false);
        if (descriptor == 0) continue;

        if (free) {
            kfree((void *)(descriptor->phys_addr << 12));
        }

        descriptor->present = false;
    }

    return 0;
}

void *map_mmio(vmm_info_t *vmm, u64 physical, size_t size, bool writable) {
    if (vmm == NULL) vmm = &kernel_vmm;

    u8 flag = 0;
    if (writable) flag |= MEMORY_FLAG_WRITE;
    if (vmm->user_vmm) flag |= MEMORY_FLAG_USER;
    memory_area_t *area = vmm_alloc(vmm, size, flag);
    int result = mappages(vmm->root_pagetable, (void *)area->start, area->size,
                          (void *)physical, area->flags);
    if (result) {
        logf(ERROR, "Error while mapping IO");
        return NULL;
    };

    return (void *)area->start;
}

void map_higher_half(void *pt_addr) {
    pml4e_t *pagetable = (pml4e_t *)pt_addr;
    for (u64 i = 256; i < 512; i++) {
        memcpy(&pagetable[i], &kernel_pml4[i], sizeof(pml4e_t));
    }
}
