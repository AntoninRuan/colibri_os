#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

struct heap_node {
    size_t size;
#define FREE 0
#define USED 1
    uint8_t status;
    struct heap_node *prev;
    struct heap_node *next;
};

typedef struct heap_node heap_node_t;

extern heap_node_t *kernel_heap;

void init_heap(heap_node_t *, size_t);
void *alloc(heap_node_t *, size_t);
void free(void *);

#endif  // HEAP_H
