#include <kernel/kernel.h>
#include <kernel/list.h>

void lst_init(struct lst *lst) {
    lst->next = lst;
    lst->prev = lst;
}

bool lst_empty(struct lst *lst) { return lst->next == lst; }

void lst_push(struct lst *lst, void *p) {
    struct lst *n = (struct lst *)p;
    n->next = lst->next;
    n->prev = lst;

    lst->next->prev = n;
    lst->next = n;
}

void lst_push_end(struct lst *lst, void *p) {
    struct lst *n = (struct lst *)p;

    n->prev = lst->prev;
    n->next = lst;

    lst->prev->next = n;
    lst->prev = n;
}

void *lst_pop(struct lst *lst) {
    if (lst->next == lst) {
        panic("empty struct lst");
    }

    struct lst *n = lst->next;
    n->next->prev = lst;
    lst->next = n->next;

    return (void *)n;
}

void *lst_pop_end(struct lst *lst) {
    if (lst->prev == lst) {
        panic("empty struct lst");
    }

    struct lst *p = lst->prev;
    p->prev->next = lst;
    lst->prev = p->prev;
    return (void *)p;
}
