#ifndef PHYSICAL_ALLOCATOR_H
#define PHYSICAL_ALLOCATOR_H

#include <kernel/memory/vmm.h>

void init_phys_allocator(memory_area_t *ram_available);
void* kalloc();
void kfree(void *);

#endif // PHYSICAL_ALLOCATOR_H
