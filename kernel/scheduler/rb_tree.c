#include <kernel/memory/heap.h>
#include <kernel/scheduler.h>
#include <kernel/scheduler/rb_tree.h>
#include <stdbool.h>

void rotate(sched_tree_t *t, rb_node_t *n, dir_t dir) {
    rb_node_t *p = n->parent;
    rb_node_t *root = n->child[1 - dir];
    rb_node_t *child = root->child[dir];

    n->child[1 - dir] = child;
    if (child) child->parent = n;

    root->parent = p;
    if (p)
        p->child[DIR(n)] = root;
    else
        t->root = root;

    n->parent = root;
    root->child[dir] = n;
}

void balance_tree_insert(sched_tree_t *t, rb_node_t *n, rb_node_t *parent) {
    // n is the root of tree
    if (!parent) {
        n->color = BLACK;
        return;
    }

    do {
        // Tree is already balanced
        if (parent->color == BLACK) return;

        rb_node_t *gp = parent->parent;

        // parent is the root
        if (!gp) {
            parent->color = BLACK;
            return;
        }

        dir_t dir = DIR(parent);
        rb_node_t *uncle = gp->child[1 - dir];
        if (!uncle || uncle->color == BLACK) {
            if (n == parent->child[1 - dir]) {
                // Internal case dir(n) != dir(p)
                // Getting back to the case dir(n) == dir(p)
                rotate(t, parent, dir);
                n = parent;
                parent = gp->child[dir];
            }

            // External case dir(n) == dir(p)
            rotate(t, gp, 1 - dir);
            gp->color = RED;
            parent->color = BLACK;
            return;
        }

        // Uncle and parent are both red, gp is necessary black. We can make the
        // subtree gp valid by making parent and black and gp red. But we keep
        // iterating because we might have broken t by making gp red.
        parent->color = BLACK;
        uncle->color = BLACK;
        gp->color = RED;
        n = gp;

    } while ((parent = n->parent));

    return;
}

void rb_insert(sched_tree_t *rb, sched_entity_t *se) {
    rb_node_t **link = &rb->root;
    rb_node_t *parent = NULL;
    u64 key = entity_key(se);
    bool leftmost = true;
    rb_node_t *run = alloc(sizeof(rb_node_t));
    run->color = RED;
    run->left = (run->right = NULL);
    run->se = se;

    while (*link) {
        parent = *link;
        if (key < entity_key(parent->se)) {
            link = &parent->left;
        } else {
            link = &parent->right;
            leftmost = false;
        }
    }

    if (leftmost) {
        rb->leftmost = run;
    }

    run->parent = parent;
    *link = run;
    balance_tree_insert(rb, run, parent);
}

void balance_tree_remove(sched_tree_t *t, rb_node_t *n) {
    rb_node_t *p;
    rb_node_t *s;
    rb_node_t *c;
    rb_node_t *d;

    do {
        dir_t dir = DIR(n);
        p = n->parent;
        s = p->child[1 - dir];
        c = s->child[dir];
        d = s->child[1 - dir];

        // Case 3
        // If s is red necessarily p, c and d are black
        if (s->color == RED) {
            rotate(t, p, dir);
            p->color = RED;
            s->color = BLACK;
            s = c;
            c = s->child[dir];
            d = s->child[1 - dir];
        }

        // At this point s is black
        if (!d || d->color == BLACK) {
            if (!c || c->color == BLACK) {
                s->color = RED;
                if (p->color == BLACK) {
                    // Case 2
                    continue;
                } else {
                    // Case 4
                    p->color = BLACK;
                    return;
                }
            } else {
                // Case 5
                rotate(t, s, 1 - dir);
                c->color = BLACK;
                s->color = RED;
                d = s;
                s = c;
            }
        }

        // Case 6
        rotate(t, p, dir);
        s->color = p->color;
        p->color = BLACK;
        d->color = BLACK;
        return;
    } while ((n = n->parent) && n->parent);

    // Case 1
    t->root = n;
    return;
}

rb_node_t *leftmost(rb_node_t *sub) {
    while (sub->left) sub = sub->left;
    return sub;
}

rb_node_t *rightmost(rb_node_t *sub) {
    while (sub->right) sub = sub->right;
    return sub;
}

void rb_remove(sched_tree_t *rb, sched_entity_t *se) {
    rb_node_t *n = rb->root;
    u64 key = entity_key(se);

    while (n && n->se != se) {
        if (key < entity_key(n->se)) {
            n = n->left;
        } else {
            n = n->right;
        }
    }

    // se was not in the tree
    if (!n) return;

    // n has 2 children, swapping it with its immediate successor in order to
    // have n with 0 or only a right child
    if (n->left && n->right) {
        rb_node_t *sw = rightmost(n->left);
        n->se = sw->se;
        n = sw;
    }

    // n has only 1 child which must be red, swapping both turning its child
    // black and removing n
    if (n->left || n->right) {
        rb_node_t *child;
        if (n->left)
            child = n->left;
        else
            child = n->right;
        dir_t dir = DIR(child);
        n->se = child->se;
        n->child[dir] = NULL;
        if (child == rb->leftmost) rb->leftmost = n;
        free(child);
        return;
    }

    // Now n has no child
    if (n == rb->leftmost) rb->leftmost = n->parent;

    if (n == rb->root) {
        rb->root = NULL;
        free(n);
        return;
    }

    dir_t dir = DIR(n);
    if (n->color == RED) {
        n->parent->child[dir] = NULL;
        free(n);
        return;
    }

    balance_tree_remove(rb, n);
    n->parent->child[dir] = NULL;
    free(n);
}
