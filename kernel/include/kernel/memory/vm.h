#ifndef VM_H
#define VM_H

#include <kernel/memory/vmm.h>
#include <kernel/multiboot2.h>
#include <stddef.h>
#include <stdint.h>

void kvminit(struct multiboot_memory_map *);
void clean_paging();
void *walk(void *pagetable, void *va, bool alloc);
int mappages(void *pagetable, void *va, u64 sz, void *pa, u8 flags);
int updatepages(void *pagetable, void *va, u64 sz, u8 flags);
int unmappages(void *pagetable, void *va, u64 sz, bool free);
void *map_mmio(vmm_info_t *, u64 physical, size_t size, bool writable);
void map_higher_half(void *pagetable);

#endif  // VM_H
