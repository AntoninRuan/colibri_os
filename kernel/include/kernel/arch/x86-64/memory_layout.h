#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

#include <stdint.h>

#define SMALL_PAGE_SIZE  0x1000     // 4K
#define MEDIUM_PAGE_SIZE 0x200000   // 2M = 4K * 512
#define BIG_PAGE_SIZE    0x40000000 // 1G = 2M * 512

// Only works for symbols in the higher half kernel
#define PHYSICAL_ADDRESS(symbol) ((uint64_t) &symbol - (uint64_t) &_kernel_virtual_offset)

// pml4[509]
#define PHYSICAL_OFFSET 0xFFFFFE8000000000

#define VA2INDEX(va, level) (((uint64_t) va >> (12 + level * 9)) & 0x1FF)
#define VA2PML4_INDEX(va)  VA2INDEX(va, 3)
#define VA2PDPT_INDEX(va)  VA2INDEX(va, 2)
#define VA2PD_INDEX(va)    VA2INDEX(va, 1)
#define VA2PT_INDEX(va)    VA2INDEX(va, 0)

#define PAGE_START(addr, size) ((uint64_t) (addr) - ((uint64_t)(addr) % (uint64_t) size))
#define PAGE_END(addr, size) (PAGE_START(addr, size) + size - 1)

#define KERNEL_P3_HH 511
#define KERNEL_MMIO 511
#define PML4_RECURSE_ENTRY 510

extern uint8_t _kernel_virtual_offset;
extern uint8_t _kernel_virtual_end;

typedef union pml4e_t {
    struct {
        uint64_t present   : 1;  // Bit 0
        uint64_t writable  : 1;  // Bit 1
        uint64_t user_page : 1;  // Bit 2: If set has user level else supervisor
        uint64_t pwt       : 1;  // Bit 3
        uint64_t pcd       : 1;  // Bit 4
        uint64_t accessed  : 1;  // Bit 5
        uint64_t ignored_1 : 1;  // Bit 6
        uint64_t           : 1;  // Bit 7
        uint64_t global    : 1;  // Bit 8: Ignored if PS is not set
        uint64_t ignored_2 : 2;  // Bit 9..10
        uint64_t restart   : 1;  // Bit 11: Used only if HLAT paging is enabled
        // Physical address 4KB aligned of the pdpt referenced
        uint64_t pdpt_addr   : 39; // Bit 12..50
        uint64_t           : 1;  // Bit 51
        uint64_t ignored_3 : 11;  // Bit 52..62
        uint64_t xd        : 1;  // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    uint64_t raw;
} pml4e_t;

typedef union pdpte_t {
    struct {
        uint64_t present   : 1;  // Bit 0
        uint64_t writable  : 1;  // Bit 1
        uint64_t user_page : 1;  // Bit 2: If set has user level else supervisor
        uint64_t pwt       : 1;  // Bit 3
        uint64_t pcd       : 1;  // Bit 4
        uint64_t accessed  : 1;  // Bit 5
        uint64_t dirty     : 1;  // Bit 6: Ignored if PS is not set
        uint64_t page_size : 1;  // Bit 7: If set ref a 1G page else a PD
                                 // If it is a PDE 2M page
        uint64_t global    : 1;  // Bit 8: Ignored if PS is not set
        uint64_t ignored   : 2;  // Bit 9..10
        uint64_t restart   : 1;  // Bit 11: Used only if HLAT paging is enabled
        /* If PS is set:
         *     - pd_addr.0 is PAT bit
         *     - pd_addr.1..17 are reserved and must be 0
         *     For a PDE only pd_addr.1..8 are reserved
         *     - pd_addr.18..38 are physical addr of the start of the page
         * Else:
         *     - pd.addr is the physical address 4KB aligned of the pd referenced
         */
        uint64_t pd_addr   : 39; // Bit 12..50
        uint64_t           : 1;  // Bit 51
        uint64_t ignored_2 : 7;  // Bit 52..58
        uint64_t protect_key : 4;// Bit 59..62 Ignored if PS is not set
        uint64_t xd        : 1;  // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    uint64_t raw;
} pdpte_t;

typedef pdpte_t pde_t;

typedef union pte_t {
    struct {
        uint64_t present   : 1;  // Bit 0
        uint64_t writable  : 1;  // Bit 1
        uint64_t user_page : 1;  // Bit 2: If set has user level else supervisor
        uint64_t pwt       : 1;  // Bit 3
        uint64_t pcd       : 1;  // Bit 4
        uint64_t accessed  : 1;  // Bit 5
        uint64_t dirty     : 1;  // Bit 6
        uint64_t pat       : 1;  // Bit 7
        uint64_t global    : 1;  // Bit 8: Ignored if PS is not set
        uint64_t ignored   : 2;  // Bit 9..10
        uint64_t restart   : 1;  // Bit 11: Used only if HLAT paging is enabled
        uint64_t phys_addr   : 39; // Bit 12..50
        uint64_t           : 1;  // Bit 51
        uint64_t ignored_2 : 7;  // Bit 52..58
        uint64_t protect_key : 4;// Bit 59..62 Ignored if PS is not set
        uint64_t xd        : 1;  // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    uint64_t raw;
} pte_t;

typedef struct memory_area_t memory_area_t;
struct memory_area_t {
    uint64_t start;
    uint64_t size;
    uint8_t flags;
    memory_area_t *next;
};

#define MEMORY_FLAG_WRITE 1
#define MEMORY_FLAG_USER  (1 << 1)
#define MEMORY_FLAG_EXEC  (1 << 2)

#endif // MEMORY_LAYOUT_H
