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

#include "test.h"

void *alloc(u64 sz) { return malloc(sz); }

bool is_valid_tree(rb_node_t *n, int *height) {
    if (!n) return true;
    u64 key = entity_key(n->se);

    int h_l = 0, h_r = 0;
    if (n->left) {
        if (key < entity_key(n->left->se)) return false;
        if (n->color == RED && n->left->color == RED) return false;
        if (!is_valid_tree(n->left, &h_l)) return false;
    }
    if (n->right) {
        if (key > entity_key(n->right->se)) return false;
        if (n->color == RED && n->right->color == RED) return false;
        if (!is_valid_tree(n->right, &h_r)) return false;
    }

    if (height && n->color == BLACK) (*height)++;

    return true;
}

char *color[2] = {"red", "black"};

void dump_tree(rb_node_t *n, bool first, FILE *f) {
    rb_node_t *root = n;
    if (first) {
        f = fopen("tree.dot", "w");
        fprintf(f, "digraph G {\n");
    }

    fprintf(f, "\"%ld\" [color=%s]\n", n->se->vruntime, color[n->color]);
    if (n->left) {
        fprintf(f, "\"%ld\" -> \"%ld\"\n", n->se->vruntime,
                n->left->se->vruntime);
        dump_tree(n->left, false, f);
    }
    if (n->right) {
        fprintf(f, "\"%ld\" -> \"%ld\"\n", n->se->vruntime,
                n->right->se->vruntime);
        dump_tree(n->right, false, f);
    }

    if (first) {
        fprintf(f, "}\n");
        fclose(f);
    }

    return;
}

#define N 10000
u32 insert_list[N];
u32 remove_perm[N];

u32 random_range(u32 a, u32 b) {
    int r = rand();
    int d = b - a;
    return (r % d) + a;
}

void shuffle(u32 *l, size_t s) {
    srand(5465451);
    for (int i = 0; i < s - 1; i++) {
        u32 j = random_range(i, s);
        u32 tmp = l[i];
        l[i] = l[j];
        l[j] = tmp;
    }
}

void generate_test_list() {
    srand(5);
    for (u32 i = 0; i < N; i++) {
        insert_list[i] = rand();
    }

    for (u32 i = 0; i < N; i++) {
        remove_perm[i] = i;
    }

    shuffle(remove_perm, N);
}

int main(void) {
    sched_tree_t t = {.root = NULL, .leftmost = NULL};

    sched_entity_t *ents = calloc(N, sizeof(sched_entity_t));
    generate_test_list();

    for (u32 i = 0; i < N; i++) {
        ents[i].vruntime = insert_list[i];
    }

    for (u32 i = 0; i < N; i++) {
        rb_insert(&t, &ents[i]);
        if (!is_valid_tree(t.root, NULL)) {
            log_test(ERROR, "rb_insert");
            return 1;
        }
    }

    log_test(OK, "rb_insert");

    for (u32 i = 0; i < N; i++) {
        rb_remove(&t, &ents[remove_perm[i]]);
        if (!is_valid_tree(t.root, NULL)) {
            log_test(ERROR, "rb_remove");
            return 1;
        }
    }

    log_test(OK, "rb_remove");

    return 0;
}
