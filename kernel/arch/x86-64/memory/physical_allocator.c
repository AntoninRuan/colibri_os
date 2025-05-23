#include <kernel/arch/x86-64/memory_layout.h>
#include <kernel/kernel.h>
#include <kernel/list.h>
#include <kernel/log.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/sync.h>
#include <stdint.h>
#include <string.h>

typedef struct lst page_lst;

static u64 base;
static char *alloc;
static u64 alloc_size;
page_lst free_lst = {0};
spinlock_t phys_alloc_lock = {.name = "Physical Allocator"};

// void lst_init(page_lst *lst) {
//     lst->next = lst;
//     lst->prev = lst;
// }

// bool lst_empty(page_lst *lst) { return lst->next == lst; }

// void lst_push(page_lst *lst, void *p) {
//     page_lst *n = (page_lst *)(p + PHYSICAL_OFFSET);
//     n->next = lst->next;
//     n->prev = lst;

//     lst->next->prev = n;
//     lst->next = n;
// }

// void lst_push_end(page_lst *lst, void *p) {
//     page_lst *n = (page_lst *)(p + PHYSICAL_OFFSET);

//     n->prev = lst->prev;
//     n->next = lst;

//     lst->prev->next = n;
//     lst->prev = n;
// }

// void *lst_pop(page_lst *lst) {
//     if (lst->next == lst) {
//         panic("empty page_lst");
//     }

//     page_lst *n = lst->next;
//     n->next->prev = lst;
//     lst->next = n->next;

//     return ((void *)n) - PHYSICAL_OFFSET;
// }

// Return page index of a given physical address
u64 page_index(u64 addr) {
    u64 pstart = PAGE_START(addr, PAGE_SIZE);
    u64 offset = pstart - base;
    return (u64)(offset / PAGE_SIZE);
}

u64 addr(u64 index) { return base + index * PAGE_SIZE; }

bool bit_isset(u64 index) {
    char c = alloc[index / 8];
    return (c & (1L << (index % 8)));
}

void bit_set(u64 index) { alloc[index / 8] |= 1L << (index % 8); }

void bit_clear(u64 index) { alloc[index / 8] &= ~(u8)(1 << (index % 8)); }

void init_phys_allocator(memory_area_t *ram_available) {
    logf(INFO, "Init physical allocator at base=0x%X with size 0x%X",
         ram_available->start, ram_available->size);
    lst_init(&free_lst);
    base = PAGE_END(ram_available->start, PAGE_SIZE) + 1;
    u64 page_count = (ram_available->size / PAGE_SIZE);
    if (page_count == 0) return;

    alloc = (char *)(base + PHYSICAL_OFFSET);
    alloc_size = ((page_count - 1) / 8) + 1;
    memset(alloc, 0, alloc_size);

    u64 index = page_index(base + alloc_size - 1) + 1;
    u64 i = 0;
    for (; i < index; i++) {
        bit_set(i);
    }

    for (; i < page_count; i++) {
        lst_push_end(&free_lst,
                     (void *)(base + i * PAGE_SIZE) + PHYSICAL_OFFSET);
    }

    for (; i < alloc_size * 8; i++) {
        bit_set(i);
    }
}

// Always allocate 4 kB pages
// Return the physical address of the page start
void *kalloc() {
    acquire(&phys_alloc_lock);
    if (lst_empty(&free_lst)) return 0;
    void *page = lst_pop(&free_lst) - PHYSICAL_OFFSET;
    bit_set(page_index((u64)page));
    release(&phys_alloc_lock);
    return page;
}

void kfree(void *page) {
    u64 addr = PAGE_START((u64)page, PAGE_SIZE);

    u64 index = page_index(addr);

    // Page is already free
    if (!bit_isset(index)) return;

    acquire(&phys_alloc_lock);
    lst_push(&free_lst, (void *)addr + PHYSICAL_OFFSET);
    bit_clear(index);
    release(&phys_alloc_lock);
}
