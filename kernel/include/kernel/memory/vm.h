#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/multiboot2.h>
#include <kernel/memory/vmm.h>

#ifdef __arch_x86_64
#include <kernel/arch/x86-64/memory_layout.h>
#endif

void kvminit(struct multiboot_memory_map *);
void clean_paging();
void* walk(pml4e_t *pagetable, void *va, bool alloc);
int mappages(pml4e_t *pagetable, void *va, uint64_t sz, void *pa,
             uint8_t flags);
int unmappages(pml4e_t *pagetable, void *va, uint64_t sz, bool free);
void* map_mmio(vmm_info_t *, uint64_t physical, size_t size, bool writable);

#endif // VM_H
