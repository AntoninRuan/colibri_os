#include <kernel/multiboot2.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/arch/x86-64/vm.h>
#include <kernel/arch/x86-64/physical_allocator.h>

alignas(4096) pdpte_t physical_mapping[512] = {0}; // Goes to pml4[509]

uint64_t get_va(uint16_t pml4_e, uint16_t pdpt_e, uint16_t pd_e, uint16_t pt_e, uint32_t offset) {
    uint64_t addr = 0;

    bool small_page = (bool) (pt_e < 512);
    if (pml4_e & 0x100)
        addr |= 0xFFFFL << 48;

    addr |= (uint64_t) pml4_e << 39;
    addr |= (uint64_t) pdpt_e << 30;
    addr |= (uint64_t) pd_e << 21;
    if (small_page)
        addr |= (uint64_t) pt_e << 12;
    addr |= offset;

    return addr;
}

uint64_t pdpt_va(uint16_t pml4_offset) {
    return get_va(PML4_RECURSE_ENTRY,
                  PML4_RECURSE_ENTRY,
                  PML4_RECURSE_ENTRY,
                  pml4_offset, 0);
}

uint64_t pd_va(uint16_t pml4_offset, uint16_t pdpt_offset) {
    return get_va(PML4_RECURSE_ENTRY,
                  PML4_RECURSE_ENTRY,
                  pml4_offset,
                  pdpt_offset, 0);
}

uint64_t pt_va(uint16_t pml4_offset, uint16_t pdpt_offset, uint16_t pd_offset) {
    return get_va(PML4_RECURSE_ENTRY,
                  pml4_offset,
                  pdpt_offset,
                  pd_offset, 0);
}

memory_area_t ram_available = {0};

void kvminit(struct multiboot_memory_map *mmap) {
    // Maps 512GB of physical memory for easy access in allocators
    for (int i = 0; i < 512; i ++) {
        pdpte_t entry = {0};
        entry.raw = (uint64_t) BIG_PAGE_SIZE * i;
        entry.present = true;
        entry.writable = true;
        entry.page_size = 1;
        physical_mapping[i] = entry;
    }
    pml4e_t physical_entry = {0};
    physical_entry.raw = (uint64_t)(PHYSICAL_ADDRESS(physical_mapping));
    physical_entry.present = true;
    physical_entry.writable = true;
    ((pml4e_t *)(pdpt_va(PML4_RECURSE_ENTRY)))[509] = physical_entry;

    // Determine RAM area
    uint64_t max_ram_base_addr = 0;
    uint64_t max_ram_size = 0;
    struct multiboot_mmap_entry entry;
    uint32_t entry_number = (mmap->size - 16) / mmap->entry_size;
    for (uint32_t i = 0; i < entry_number; i ++) {
        entry = mmap->entries[i];
        if (entry.type == 1 && entry.length > max_ram_size) {
            max_ram_size = entry.length;
            max_ram_base_addr = entry.base_addr;
        }
    }

    if (!max_ram_size)
        panic("kvminit: no ram detected");

    // uint64_t phys_memory_start = max_ram_base_addr;
    uint64_t phys_memory_end = max_ram_base_addr + max_ram_size - 1;

    uint64_t kernel_physical_end = PHYSICAL_ADDRESS(_kernel_virtual_end);
    ram_available.start = PAGE_END(kernel_physical_end, SMALL_PAGE_SIZE) + 1;
    ram_available.size = phys_memory_end - ram_available.start + 1;

    init_phys_allocator(&ram_available);
}
void* map_mmio(uint64_t physical, size_t size, bool writable) {
    return (void *) (physical + PHYSICAL_OFFSET);
}

void *map_memory(uint64_t pagetable, void *phys, size_t size, void* virt);
