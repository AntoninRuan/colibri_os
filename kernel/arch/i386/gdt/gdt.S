#define FLAG_G          (1 << 55) // Granularity flags, if set limit is in 64KiB blocks else in 1B blocks
#define FLAG_DB         (1 << 54) // If set descriptor defines 32-bit protected mode segments else a 16-bit protected mode
#define FLAG_L          (1 << 53) // If set descriptor defines a 64-bit code segment, if set DB should be clear

#define ACCESS_PRESENT  (1 << 47)
#define ACCESS_DPL(x)   (x << 45) // Privilege field, CPU privilege level of the segment
#define ACCESS_S        (1 << 44) // If clear defines a system segment (Task State Segment, ...) else a code/data segment
#define ACCESS_CODE     (1 << 43) // If set defines a code segment else a data one

// For data segments direction flag: if clear segment grows up else segment grows downward
// For code segments conforming flag: if clear code can only from ring == DPL, if set code can run if ring <= DPL
#define ACCESS_DC       (1 << 42)

//  For data segments write flag: if set write is authorized, execute is never granted
//  For code segments read flag: if set read is authorized, write is never granted
#define ACCESS_RW       (1 << 41)
#define ACCESS_A        (1 << 40) // Set by the CPU is segment is accessed, best to always be set

    .section .data
    .global gdtr

    // GDT is using a flat model with
    // 5 entry: the first one being the mandatory null one
    // 2 for kernel code and data
    // 2 for user code and data
gdt64:
    .quad 0

#define FLAGS (FLAG_G | FLAG_DB)
#define ACCESS_KERNEL (ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_S | ACCESS_RW | ACCESS_A)
#define ACCESS_USER (ACCESS_KERNEL | ACCESS_DPL(3))
    // Limit will always be 0xFFFFF
    // So bits 0..15 must be set
    // And also bits 48..51
#define LIMIT (0xFFFF | (0xF << 48))

    .quad (FLAGS | ACCESS_KERNEL | ACCESS_CODE | LIMIT)
    .quad (FLAGS | ACCESS_KERNEL | LIMIT)
    .quad (FLAGS | ACCESS_USER | ACCESS_CODE | LIMIT)
    .quad (FLAGS | ACCESS_USER | LIMIT)

gdtr:
    .short gdtr - gdt64 - 1
    .long gdt64
