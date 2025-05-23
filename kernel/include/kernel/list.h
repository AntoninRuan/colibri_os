#ifndef LIST_H
#define LIST_H

struct lst {
    struct lst *prev;
    struct lst *next;
};

void lst_init(struct lst *lst);
bool lst_empty(struct lst *lst);
void lst_push(struct lst *, void *p);
void lst_push_end(struct lst *, void *p);
void *lst_pop(struct lst *);
void *lst_pop_end(struct lst *);

#endif  // LIST_H
