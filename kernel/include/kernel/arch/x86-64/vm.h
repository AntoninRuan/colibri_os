#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/multiboot2.h>

void kvminit(struct multiboot_memory_map*);
void *map_mmio(uint64_t physical, size_t size, bool writable);
void unmap_mmio(void *addr, size_t size);

#endif // VM_H
