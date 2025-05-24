#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vmm.h>
#include <stddef.h>
#include <stdint.h>

heap_node_t *kernel_heap;

void init_heap(heap_node_t *heap, size_t size) {
    heap->size = size - sizeof(heap_node_t);
    heap->status = FREE;
    heap->prev = NULL;
    heap->next = NULL;
    logf(INFO, "Kernel heap initialized with size 0x%x", size);
}

void *alloc(heap_node_t *heap, size_t size) {
    if (heap == NULL) heap = kernel_heap;

    void *result = NULL;
    heap_node_t *cur = heap;

    while (cur->status == USED || cur->size < size) {
        if (cur->next == NULL) break;
        cur = cur->next;
    }

    if (cur->next == NULL && (cur->status == USED || cur->size < size)) {
        memory_area_t *area =
            vmm_alloc_at((uintptr_t)cur, &kernel_vmm, size, MEMORY_FLAG_WRITE);

        if (area == NULL)
            // Out of memory
            return NULL;

        if (area->start == (u64)cur + cur->size) {
            cur->size += area->size;
        } else {
            heap_node_t *tmp = cur;
            cur = (heap_node_t *)area->start;
            cur->size = area->size;
            cur->status = FREE;
            cur->prev = tmp;
            cur->next = NULL;

            tmp->next = cur;
        }
    }

    result = (void *)cur + sizeof(heap_node_t);

    heap_node_t *next = NULL;
    if (cur->size > size) {
        next = (heap_node_t *)(result + size);
        next->size = cur->size - size - sizeof(heap_node_t);
        next->status = FREE;
        next->prev = cur;
        next->next = cur->next;
        cur->next = next;
    }

    cur->status = USED;
    cur->size = size;

    return result;
}

void free(void *ptr) {
    heap_node_t *node = (ptr - sizeof(heap_node_t));
    if (node->status != FREE && node->status != USED) {
        // Free on an invalid address
        return;
    }

    // Node is already free
    if (node->status == FREE) return;

    node->status = FREE;

    if (node->next && node->next->status == FREE) {
        if (node->size + ptr == node->next) {
            // Merge with the right node
            heap_node_t *merging = node->next;
            if (merging->next)
                merging->next->prev = node;
            node->next = merging->next;

            node->size += merging->size + sizeof(heap_node_t);
        }
    }

    if (node->prev && node->prev->status == FREE) {
        heap_node_t *prev = node->prev;
        if ((void *)prev + prev->size + sizeof(heap_node_t) == node) {
            // Merge with the left node
            prev->next = node->next;
            node->next->prev = prev;

            prev->size += node->size + sizeof(heap_node_t);
        }
    }

    return;
}
