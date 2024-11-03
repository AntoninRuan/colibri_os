#ifndef GDT_H
#define GDT_H

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

#define GDT_ENTRY_KERNEL_CODE 0x8
#define GDT_ENTRY_KERNEL_DATA 0x10
#define GDT_ENTRY_USER_CODE   0x18
#define GDT_ENTRY_USER_DATA   0x20

struct GDTR {
    uint16_t limit;
    uint32_t addr;
} __attribute__((packed));

void load_gdt();

#endif // GDT_H
