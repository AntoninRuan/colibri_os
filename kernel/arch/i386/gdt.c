#include <stdint.h>

#define ENTRY_FLAG_G          (1 << 3) // Granularity flags, if set limit is in 64KiB blocks else in 1B blocks
#define ENTRY_FLAG_DB         (1 << 2) // If set descriptor defines 32-bit protected mode segments else a 16-bit protected mode
#define ENTRY_FLAG_L          (1 << 1) // If set descriptor defines a 64-bit code segment, if set DB should be clear

#define ENTRY_ACCESS_DPL(x)   (x << 5) // Privilege field, CPU privilege level of the segment
#define ENTRY_ACCESS_S(x)     (x << 4) // If clear defines a system segment (Task State Segment, ...) else a code/data segment
#define ENTRY_ACCESS_CODE     (1 << 3) // If set defines a code segment else a data one
// For data segments direction flag: if clear segment grows up else segment grows downward
// For code segments conforming flag: if clear code can only from ring == DPL, if set code can run if ring <= DPL
#define ENTRY_ACCESS_DC       (1 << 2)
//  For data segments write flag: if set write is authorized, execute is never granted
//  For code segments read flag: if set read is authorized, write is never granted
#define ENTRY_ACCESS_RW       (1 << 1)

uint64_t gdt_entries[];
uint64_t num_gdt_entries = 0;

struct GDTR {
    uint16_t limit;
    uint32_t addr;
} __attribute__((packed));

struct GDTR gdtr = {0};

uint64_t gdt_entry(uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
    uint64_t entry = 0;

    // Set high 32 bit of entry
    entry |= (base & 0xFF000000);       // Set base bit 31:24
    entry |= (base & 0x00FF0000) >> 16; // Set base bit 23:16

    entry |= (limit & 0x000F0000); // Set limit bit 19:16

    entry |= (flags & 0xF) << 20;
    entry |= access << 8;

    entry <<= 32;

    // Now the low 32 bit (ignored in long mode)
    entry |= (base & 0xFFFF) << 16; // Set base bit 15:0
    entry |= (limit & 0xFFFF);      // Set limit bit 15:0

    return entry;
}

void load_gdt() {
    gdt_entries[0] = 0;
    num_gdt_entries ++;

    uint64_t kernel_code = gdt_entry(0, 0xFFFF,
                                            ENTRY_FLAG_G | ENTRY_FLAG_DB,
                                            ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(0) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);


    uint64_t kernel_data = gdt_entry(0, 0xFFFF,
                                            ENTRY_FLAG_G | ENTRY_FLAG_DB,
                                            ENTRY_ACCESS_DPL(0) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    gdt_entries[1] = kernel_code;
    gdt_entries[2] = kernel_data;
    num_gdt_entries += 2;

    uint64_t user_code = gdt_entry(0, 0xFFFF, ENTRY_FLAG_G | ENTRY_FLAG_DB,
                                          ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(3) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    uint64_t user_data = gdt_entry(0, 0xFFFF, ENTRY_FLAG_G | ENTRY_FLAG_DB,
                                          ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(3) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    gdt_entries[3] = user_code;
    gdt_entries[4] = user_data;
    num_gdt_entries += 2;

    gdtr.limit = num_gdt_entries * sizeof(uint64_t) - 1;
    gdtr.addr = (uint64_t)gdt_entries;

    __asm__("lgdt %0" : : "m"(gdtr));

    return;
}
