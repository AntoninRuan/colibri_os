#define PROCESS_H
#include <stdarg.h>
#include <stdint.h>
typedef uint64_t u64;
typedef uint32_t u32;
#include <kernel/scheduler/rb_tree.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void *alloc(u64 sz) { return malloc(sz); }

bool is_valid_tree(rb_node_t *n, int *height) {
    if (!n) return true;
    u64 key = entity_key(n->se);

    int h_l = 0, h_r = 0;
    if (n->left) {
        if (key <= entity_key(n->left->se)) return false;
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

void free(void *p) { return; }

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

#include "rb_test_data.h"
#include "test.h"

int main(void) {
    sched_tree_t t = {.root = NULL, .leftmost = NULL};

    sched_entity_t ents[N] = {0};

    for (u32 i = 0; i < N; i++) {
        ents[i].vruntime = i;
    }
    u32 remove_perm[15] = {8, 0, 5, 6, 3, 11, 1, 4, 2, 14, 13, 9, 7, 12, 10};

    for (u32 i = 0; i < 15; i++) {
        rb_insert(&t, &ents[insert_list[i]]);
        if (!is_valid_tree(t.root, NULL)) {
            log_test(ERROR, "rb_insert");
            return 1;
        }
    }
    log_test(OK, "rb_insert");

    for (u32 i = 0; i < 15; i++) {
        rb_remove(&t, &ents[insert_list[remove_perm[i]]]);
        if (!is_valid_tree(t.root, NULL)) {
            log_test(ERROR, "rb_remove");
            return 1;
        }
    }

    log_test(OK, "rb_remove");

    return 0;
}
