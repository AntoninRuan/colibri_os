#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/arch/x86-64/physical_allocator.h>
#include <kernel/arch/x86-64/memory_layout.h>

struct page_lst {
    struct page_lst *next;
    struct page_lst *prev;
};

typedef struct page_lst page_lst;

uint64_t base;
char *alloc;
uint64_t alloc_size;
page_lst free = {0};

void lst_init(page_lst *lst) {
    lst->next = lst;
    lst->prev = lst;
}

bool lst_empty(page_lst *lst) { return lst->next == lst; }

void lst_push(page_lst *lst, void* p) {
    page_lst *n = (page_lst *) (p + PHYSICAL_OFFSET);
    n->next = lst->next;
    n->prev = lst;

    lst->next->prev = n;
    lst->next = n;
}

void* lst_pop(page_lst *lst) {
    if (lst->next == lst) {
        panic("empty page_lst");
    }

    page_lst *n = lst->next;
    n->next->prev = lst;
    lst->next = n->next;

    return ((void *)n) - PHYSICAL_OFFSET;
}

// Return page index of a given physical address
uint64_t page_index(uint64_t addr) {
    uint64_t pstart = PAGE_START(addr, SMALL_PAGE_SIZE);
    uint64_t offset = pstart - base;
    return (uint64_t)(offset / SMALL_PAGE_SIZE);
}

uint64_t addr(uint64_t index) {
    return base + index * SMALL_PAGE_SIZE;
}

bool bit_isset(uint64_t index) {
    char c = alloc[index / 8];
    return (c & (1L << (index % 8)));
}

void bit_set(uint64_t index) {
    alloc[index / 8] |= 1L << (index % 8);
}

void bit_clear(uint64_t index) {
    alloc[index / 8] &= ~(uint8_t)(1 << (index % 8));
}

void init_phys_allocator(memory_area_t *ram_available) {
    base = PAGE_END(ram_available->start, SMALL_PAGE_SIZE) + 1;
    uint64_t page_count = (ram_available->size / SMALL_PAGE_SIZE);

    alloc_size = (page_count / 8) + 1;
    memset(alloc, 0, alloc_size);

    uint64_t index = page_index(base + alloc_size) + 1;
    uint64_t i = 0;
    for (; i < index; i ++) {
        bit_set(i);
    }

    for (; i < page_count; i ++) {
        lst_push(&free, (void *)(base + i * SMALL_PAGE_SIZE));
    }

    for(; i < alloc_size * 8; i ++) {
        bit_set(i);
    }
}

// Always allocate 4 kB pages
// Return the physical address of the page start
void *kalloc() {
    if (lst_empty(&free)) return 0;
    void* page = lst_pop(&free);
    bit_set(page_index((uint64_t) page));
    return page;
}

void kfree(void *page) {
    uint64_t addr = PAGE_START((uint64_t) page, SMALL_PAGE_SIZE);

    uint64_t index = page_index(addr);
    // Page is already free
    if(!bit_isset(index)) return;

    lst_push(&free, (void *) addr);
    bit_clear(index);
}
