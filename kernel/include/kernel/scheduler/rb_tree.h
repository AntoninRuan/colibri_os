#ifndef R_TREE_H
#define R_TREE_H

#include <kernel/scheduler.h>

typedef struct rb_node rb_node_t;

typedef enum { RED, BLACK } rb_col;

struct rb_node {
    union {
        struct {
            rb_node_t *left;
            rb_node_t *right;
        };
        rb_node_t *child[2];
    };
    rb_node_t *parent;
    rb_col color;
    sched_entity_t *se;
};

typedef enum { LEFT, RIGHT } dir_t;
#define DIR(N) (N == N->parent->right ? RIGHT : LEFT)

typedef struct {
    rb_node_t *root;
    rb_node_t *leftmost;
} sched_tree_t;

void rb_remove(sched_tree_t *rb, sched_entity_t *se);
void rb_insert(sched_tree_t *rb, sched_entity_t *se);

#endif  // R_TREE_H
