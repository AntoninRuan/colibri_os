#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vmm.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

heap_node_t *kernel_heap;

void init_heap(heap_node_t *heap, size_t size) {
    heap->size = size - sizeof(heap_node_t);
    heap->status = FREE;
    heap->prev = NULL;
    heap->next = NULL;
    logf(INFO, "Kernel heap initialized with size 0x%x", size);
}

void *alloc(size_t size) {
    heap_node_t *heap = kernel_heap;

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
    if (cur->size > size + sizeof(heap_node_t)) {
        next = (heap_node_t *)(result + size);
        next->size = cur->size - size - sizeof(heap_node_t);
        next->status = FREE;
        next->prev = cur;
        next->next = cur->next;
        cur->next = next;
        cur->size = size;
    }

    cur->status = USED;

    return result;
}

// Attempt to merge node with the following one
void merge_right(heap_node_t *node) {
    void *ptr = (void *)node + sizeof(heap_node_t);
    if (node->next && node->next->status == FREE) {
        if (node->size + ptr == node->next) {
            // Merge with the right node
            heap_node_t *merging = node->next;
            if (merging->next) merging->next->prev = node;
            node->next = merging->next;

            node->size += merging->size + sizeof(heap_node_t);
        }
    }
}

void *realloc(void *ptr, size_t sz) {
    if (ptr == NULL) return alloc(sz);
    if (sz == 0) {
        free(ptr);
        return NULL;
    }

    heap_node_t *node = (heap_node_t *)(ptr - sizeof(heap_node_t));

    if (sz < node->size) {
        if (node->size - sz < sizeof(heap_node_t)) return ptr;

        heap_node_t *freed = (heap_node_t *)(ptr + sz);
        freed->status = FREE;
        freed->next = node->next;
        freed->prev = node;
        freed->size = node->size - sz - sizeof(heap_node_t);

        node->size = sz;
        if (node->next) node->next->prev = freed;
        node->next = freed;

        merge_right(freed);
        return ptr;
    }

    if (!node->next || node->next->status != FREE
        || node->size + ptr != node->next
        || node->next->size + node->size < sz) {
        void *new_ptr = alloc(sz);
        memmove(new_ptr, ptr, node->size);
        free(ptr);
        return new_ptr;
    }

    heap_node_t *right = (heap_node_t *)(ptr + sz);
    right->status = FREE;
    right->size = node->size + node->next->size - sz;
    right->next = node->next->next;
    right->prev = node;

    node->next = right;
    node->size = sz;
    if (right->next)
        right->next->prev = right;

    return ptr;
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
    merge_right(node);

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
