#include <stdint.h>
#include <stdlib.h>

#include "gdt.h"

#define GDT_ENTRY_NUM 5

uint64_t gdt_entries[GDT_ENTRY_NUM];

struct GDTR gdtr = {0};

void set_gdt_entry(uint64_t entry_selector, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
    uint8_t index = entry_selector / 8;

    if (index >= GDT_ENTRY_NUM) {
        abort();
    }

    uint64_t entry = 0;

    // Set high 32 bit of entry
    entry |= (base & 0xFF000000);       // Set base bit 31:24
    entry |= (base & 0x00FF0000) >> 16; // Set base bit 23:16

    entry |= (limit & 0x000F0000); // Set limit bit 19:16

    entry |= (flags & 0xF) << 20;
    entry |= (access | 0x80) << 8; // | 0x80 to add present flag

    entry <<= 32;

    // Now the low 32 bit (ignored in long mode)
    entry |= (base & 0xFFFF) << 16; // Set base bit 15:0
    entry |= (limit & 0xFFFF);      // Set limit bit 15:0

    gdt_entries[index] = entry;

    return;
}

void load_gdt() {
    gdt_entries[0] = 0;

    set_gdt_entry(GDT_ENTRY_KERNEL_CODE, 0, 0xFFFF,
                  ENTRY_FLAG_G | ENTRY_FLAG_DB,
                  ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(0) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    set_gdt_entry(GDT_ENTRY_KERNEL_DATA, 0, 0xFFFF,
                  ENTRY_FLAG_G | ENTRY_FLAG_DB,
                  ENTRY_ACCESS_DPL(0) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    set_gdt_entry(GDT_ENTRY_USER_CODE, 0, 0xFFFF, ENTRY_FLAG_G | ENTRY_FLAG_DB,
                  ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(3) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    set_gdt_entry(GDT_ENTRY_USER_DATA, 0, 0xFFFF, ENTRY_FLAG_G | ENTRY_FLAG_DB,
                  ENTRY_ACCESS_CODE | ENTRY_ACCESS_DPL(3) | ENTRY_ACCESS_S(1) | ENTRY_ACCESS_RW);

    gdtr.limit = GDT_ENTRY_NUM * sizeof(uint64_t) - 1;
    gdtr.addr = (uint64_t)gdt_entries;

    __asm__ volatile("lgdt %0" : : "m"(gdtr));
    __asm__ volatile("call flush_gdt");

    return;
}
