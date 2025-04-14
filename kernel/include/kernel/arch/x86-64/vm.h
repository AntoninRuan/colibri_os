#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/multiboot2.h>
#include <kernel/arch/x86-64/memory_layout.h>

void kvminit(struct multiboot_memory_map *);
void clean_paging();
void* walk(pml4e_t *pagetable, void *va, bool alloc);
void* map_mmio(uint64_t physical, size_t size, bool writable);
int mappages(pml4e_t *pagetable, void *va, uint64_t sz, void *pa,
             uint8_t flags);

#endif // VM_H
