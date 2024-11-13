#include <kernel/multiboot2.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <kernel/kernel.h>
#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/arch/x86-64/vm.h>

uint64_t phys_memory_start;
uint64_t phys_memory_end;

#define SMALL_PAGE_SIZE  0x1000    // 4K
#define MEDIUM_PAGE_SIZE 0x200000  // 2M = 4K * 512
#define HUGUE_PAGE_SIZE 040000000  // 1G = 2M * 512

// Only works for symbols in the higher half kernel
#define PHYSICAL_ADDRESS(symbol) ((uint64_t) &symbol - (uint64_t) &_kernel_virtual_offset)

#define PML4_ENTRY(addr) (((uint64_t) addr >> 39) & 0x1FF)
#define PDPT_ENTRY(addr) (((uint64_t) addr >> 30) & 0x1FF)
#define PD_ENTRY(addr)   (((uint64_t) addr >> 21) & 0x1FF)
#define PT_ENTRY(addr)   (((uint64_t) addr >> 12) & 0x1FF)

#define PAGE_START(addr, size) ((uint64_t) (addr) - ((uint64_t)(addr) % (uint64_t) size))
#define PAGE_END(addr, size) (PAGE_START(addr, size) + size - 1)

#define KERNEL_P3_HH 511
#define KERNEL_MMIO 511
#define PML4_RECURSE_ENTRY 510

extern uint8_t _kernel_virtual_offset;
alignas(4096) pde_t kernel_mmio[512] = {0}; // should be put @ kernel_p3_hh[511]


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

void kvminit(struct multiboot_memory_map *mmap) {
    // Create page dir for mmio pages for the kernel
    pdpte_t mmio;
    mmio.raw = PHYSICAL_ADDRESS(kernel_mmio);
    mmio.present = true;
    mmio.writable = true;
    pdpte_t *kernel_p3_hh = (pdpte_t *) pdpt_va(KERNEL_P3_HH);
    kernel_p3_hh[511] = mmio;
}

// TODO add check if map already exists
// TODO better allocation algorithm
uint16_t first_free_page_mmio = 0;

void *map_mmio(uint64_t physical, size_t size, bool writable) {
    uint64_t offset = physical % MEDIUM_PAGE_SIZE;
    uint64_t start = physical - offset;
    uint64_t page_needed = ((PAGE_START(physical + size, MEDIUM_PAGE_SIZE) - start) / MEDIUM_PAGE_SIZE) + 1;

    if (page_needed >= 512) {
        return NULL;
    }

    uint64_t jump_offset = 1L << ceillog2(page_needed);
    bool enough_space;
    uint64_t i;
    for (i = first_free_page_mmio; i < 512; i += jump_offset) {
        enough_space = true;
        for (uint64_t j = 0; j < page_needed; j ++) {
            if (kernel_mmio[i + j].present) {
                enough_space = false;
                break;
            }
        }
        if (enough_space) break;
    }

    if(!enough_space) return NULL;

    for(uint64_t j = 0; j < page_needed; j++) {
        kernel_mmio[i + j].raw = start + MEDIUM_PAGE_SIZE * j;
        kernel_mmio[i + j].present = true;
        kernel_mmio[i + j].writable = writable;
        kernel_mmio[i + j].page_size = true; // Allocate 2M page
    }

    if (i == first_free_page_mmio)
        first_free_page_mmio += page_needed;

    return (void *)(get_va(KERNEL_P3_HH, KERNEL_MMIO, i, -1,offset));
}

void unmap_mmio(void *addr, size_t size) {
    if (PML4_ENTRY(addr) != 511 || PDPT_ENTRY(addr) != 511) // Not a mmio mapped addr
        return;
    uint64_t first_page = PAGE_START(addr, MEDIUM_PAGE_SIZE);
    uint64_t last_page = PAGE_START(addr + size, MEDIUM_PAGE_SIZE);
    uint16_t page_freed = (last_page - first_page) / MEDIUM_PAGE_SIZE + 1;

    uint16_t first_entry = PD_ENTRY(addr);
    for (int i = 0; i < page_freed; i ++) {
        kernel_mmio[i + first_entry].present = false;
    }

    if (first_entry < first_free_page_mmio)
        first_free_page_mmio = first_entry;

    return;
}
