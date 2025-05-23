#ifndef MEMORY_LAYOUT_x86_64_H
#define MEMORY_LAYOUT_x86_64_H

#include <sys/cdefs.h>

#define SMALL_PAGE_SIZE  0x1000           // 4K
#define MEDIUM_PAGE_SIZE 0x200000         // 2M = 4K * 512
#define BIG_PAGE_SIZE    0x40000000       // 1G = 2M * 512
#define PAGE_SIZE        SMALL_PAGE_SIZE  // Default page size

#define PAGE_START(addr, size) ((u64)(addr) - ((u64)(addr) % (u64)size))
#define PAGE_END(addr, size)   (PAGE_START(addr, size) + size - 1)

// Only works for symbols in the higher half kernel
#define PHYSICAL_ADDRESS(symbol)                      \
    ((u64) & symbol - (u64) & _kernel_virtual_offset)

// pml4[509]
#define PHYSICAL_OFFSET 0xFFFFFE8000000000

#define LOWHALF_START BIG_PAGE_SIZE
#define LOWHALF_END   0x800000000000

#define VA2INDEX(va, level) (((u64)va >> (12 + level * 9)) & 0x1FF)
#define VA2PML4_INDEX(va)   VA2INDEX(va, 3)
#define VA2PDPT_INDEX(va)   VA2INDEX(va, 2)
#define VA2PD_INDEX(va)     VA2INDEX(va, 1)
#define VA2PT_INDEX(va)     VA2INDEX(va, 0)

#define KERNEL_P3_HH       511
#define KERNEL_MMIO        511
#define PML4_RECURSE_ENTRY 510

extern u8 _kernel_virtual_offset;
extern u8 _kernel_virtual_end;

static inline u64 get_va(u16 pml4_e, u16 pdpt_e, u16 pd_e, u16 pt_e,
                         u32 offset) {
    u64 addr = 0;

    bool small_page = (bool)(pt_e < 512);
    if (pml4_e & 0x100) addr |= 0xFFFFL << 48;

    addr |= (u64)pml4_e << 39;
    addr |= (u64)pdpt_e << 30;
    addr |= (u64)pd_e << 21;
    if (small_page) addr |= (u64)pt_e << 12;
    addr |= offset;

    return addr;
}

static inline u64 pdpt_va(u16 pml4_offset) {
    return get_va(PML4_RECURSE_ENTRY, PML4_RECURSE_ENTRY, PML4_RECURSE_ENTRY,
                  pml4_offset, 0);
}

static inline u64 pd_va(u16 pml4_offset, u16 pdpt_offset) {
    return get_va(PML4_RECURSE_ENTRY, PML4_RECURSE_ENTRY, pml4_offset,
                  pdpt_offset, 0);
}

static inline u64 pt_va(u16 pml4_offset, u16 pdpt_offset, u16 pd_offset) {
    return get_va(PML4_RECURSE_ENTRY, pml4_offset, pdpt_offset, pd_offset, 0);
}

#define PAGE_FLAG_MASK 0x8000000000000006

typedef union pml4e_t {
    struct {
        u64 present   : 1;  // Bit 0
        u64 writable  : 1;  // Bit 1
        u64 user_page : 1;  // Bit 2: If set has user level else supervisor
        u64 pwt       : 1;  // Bit 3
        u64 pcd       : 1;  // Bit 4
        u64 accessed  : 1;  // Bit 5
        u64 ignored_1 : 1;  // Bit 6
        u64           : 1;  // Bit 7
        u64 global    : 1;  // Bit 8: Ignored if PS is not set
        u64 ignored_2 : 2;  // Bit 9..10
        u64 restart   : 1;  // Bit 11: Used only if HLAT paging is enabled
        // Physical address 4KB aligned of the pdpt referenced
        u64 pdpt_addr : 39;  // Bit 12..50
        u64           : 1;   // Bit 51
        u64 ignored_3 : 11;  // Bit 52..62
        u64 xd        : 1;   // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    u64 raw;
} pml4e_t;

typedef union pdpte_t {
    struct {
        u64 present     : 1;  // Bit 0
        u64 writable    : 1;  // Bit 1
        u64 user_page   : 1;  // Bit 2: If set has user level else supervisor
        u64 pwt         : 1;  // Bit 3
        u64 pcd         : 1;  // Bit 4
        u64 accessed    : 1;  // Bit 5
        u64 dirty       : 1;  // Bit 6: Ignored if PS is not set
        u64 page_size   : 1;  // Bit 7: If set ref a 1G page else a PD
                              // If it is a PDE 2M page
        u64 global      : 1;  // Bit 8: Ignored if PS is not set
        u64 ignored     : 2;  // Bit 9..10
        u64 restart     : 1;  // Bit 11: Used only if HLAT paging is enabled
        /* If PS is set:
         *     - pd_addr.0 is PAT bit
         *     - pd_addr.1..17 are reserved and must be 0
         *     For a PDE only pd_addr.1..8 are reserved
         *     - pd_addr.18..38 are physical addr of the start of the page
         * Else:
         *     - pd.addr is the physical address 4KB aligned of the pd
         * referenced
         */
        u64 pd_addr     : 39;  // Bit 12..50
        u64             : 1;   // Bit 51
        u64 ignored_2   : 7;   // Bit 52..58
        u64 protect_key : 4;   // Bit 59..62 Ignored if PS is not set
        u64 xd          : 1;   // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    u64 raw;
} pdpte_t;

typedef pdpte_t pde_t;

typedef union pte_t {
    struct {
        u64 present     : 1;   // Bit 0
        u64 writable    : 1;   // Bit 1
        u64 user_page   : 1;   // Bit 2: If set has user level else supervisor
        u64 pwt         : 1;   // Bit 3
        u64 pcd         : 1;   // Bit 4
        u64 accessed    : 1;   // Bit 5
        u64 dirty       : 1;   // Bit 6
        u64 pat         : 1;   // Bit 7
        u64 global      : 1;   // Bit 8: Ignored if PS is not set
        u64 ignored     : 2;   // Bit 9..10
        u64 restart     : 1;   // Bit 11: Used only if HLAT paging is enabled
        u64 phys_addr   : 39;  // Bit 12..50
        u64             : 1;   // Bit 51
        u64 ignored_2   : 7;   // Bit 52..58
        u64 protect_key : 4;   // Bit 59..62 Ignored if PS is not set
        u64 xd          : 1;   // Bit 63 If EFER.NXE = 1, execute disable
    } __attribute__((packed));
    u64 raw;
} pte_t;

void disable_id_mapping();

#endif  // MEMORY_LAYOUT_x86_64_H
