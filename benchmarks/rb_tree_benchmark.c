#define PROCESS_H
#include <stdarg.h>
#include <stdint.h>
typedef uint64_t u64;
typedef uint32_t u32;
#include <kernel/scheduler.h>
#include <kernel/scheduler/rb_tree.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "test.h"

void *alloc(u64 sz) { return malloc(sz); }

u32 random_range(u32 a, u32 b) {
    int r = rand();
    int d = b - a;
    return (r % d) + a;
}

void shuffle(u32 *l, size_t s) {
    for (int i = 0; i < s - 1; i++) {
        u32 j = random_range(i, s);
        u32 tmp = l[i];
        l[i] = l[j];
        l[j] = tmp;
    }
}

void generate_test_list(u32 *insert_list, u32 *op_value, u32 *op_order,
                        u32 T_SIZE, u32 N) {
    srand(time(NULL));
    for (u32 i = 0; i < T_SIZE; i++) {
        insert_list[i] = rand();
    }

    for (u32 i = 0; i < N; i++) {
        op_value[i] = rand();
    }

    for (u32 i = 0; i < N; i++) {
        op_order[i] = i;
    }

    shuffle(op_order, N);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: %s <tree_size> <op_count>\n", argv[0]);
        return 1;
    }

    u32 N = atoi(argv[2]);
    u32 T_SIZE = atoi(argv[1]);

    printf("Benchmark rb_tree for tree size=%d with %d ops\n", T_SIZE, N);

    u32 *insert_list = calloc(T_SIZE, sizeof(u32));
    u32 *op_value = calloc(N, sizeof(u32));
    u32 *op_order = calloc(N, sizeof(u32));

    sched_tree_t t = {.root = NULL, .leftmost = NULL};

    sched_entity_t *ents = calloc(T_SIZE, sizeof(sched_entity_t));
    generate_test_list(insert_list, op_value, op_order, T_SIZE, N);

    printf("Filling rb_tree for benchmark\n");
    for (u32 i = 0; i < T_SIZE; i++) {
        ents[i].vruntime = insert_list[i];
    }

    // Filling tree
    for (u32 i = 0; i < T_SIZE; i++) {
        rb_insert(&t, &ents[i]);
    }

    sched_entity_t *op_ents = calloc(N, sizeof(sched_entity_t));
    for (u32 i = 0; i < N; i++) {
        op_ents[i].vruntime = op_value[i];
    }

    printf("Starting benchmark of rb_tree\n");
    struct timespec start, stop;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        log_test(ERROR, "Clock error");
        return 1;
    }

    for (u32 i = 0; i < N; i++) {
        rb_insert(&t, &op_ents[op_order[i]]);
        rb_remove(&t, &op_ents[op_order[i]]);
    }

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        log_test(ERROR, "Clock error");
        return 1;
    }

    double elapsed =
        (stop.tv_sec - start.tv_sec) + ((stop.tv_nsec - start.tv_nsec) * 1e-9F);

    printf("%.3lf\n", elapsed);
}
