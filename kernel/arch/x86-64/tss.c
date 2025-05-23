#include <kernel/arch/x86-64/tss.h>
#include <kernel/memory/heap.h>
#include <string.h>
#include <sys/cdefs.h>

typedef struct tss {
    u32 reserved0;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 io_map_base_addr;
} __attribute__((__packed__)) tss_t;

typedef struct {
    u16 limit_low;
    u16 addr_low;
    u8 addr_mid0;
    u8 access;
    u8 limit_high : 4;
    u8 flag       : 4;
    u8 addr_mid1;
    u32 addr_high;
    u32 _;
} __attribute__((__packed__)) tss_descriptor_t;

extern u8 gdt_tss_entry;
tss_t *tss;

#define FLAG_DPL(x)         (x << 5)
#define FLAG_PRESENT        (1 << 7)
#define FLAG_TYPE_TSS_AVAIL (0x9)

void setup_tss() {
    tss = alloc(NULL, sizeof(tss_t));
    memset(tss, 0, sizeof(tss_t));

    tss_descriptor_t *desc = (tss_descriptor_t *)&gdt_tss_entry;
    desc->limit_low = 0xFFFF;
    desc->limit_high = 0xF;

    desc->addr_low = (u16)((u64)tss & 0xFFFF);
    desc->addr_mid0 = (u8)(((u64)tss >> 16) & 0xFF);
    desc->addr_mid1 = (u8)(((u64)tss >> 24) & 0xFF);
    desc->addr_high = (u32)(((u64)tss >> 32) & 0xFFFFFFFF);

    desc->access = FLAG_PRESENT | FLAG_DPL(0) | FLAG_TYPE_TSS_AVAIL;
    desc->flag = (1 << 3);

    asm volatile("ltr %0" ::"r"((u16)0x28));
}

void update_rsp0(u64 rsp0) {
    if (!tss) return;
    tss->rsp0 = rsp0;
}
