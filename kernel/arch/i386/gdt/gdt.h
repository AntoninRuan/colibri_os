#ifndef GDT_H
#define GDT_H

#include <stdint.h>

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
