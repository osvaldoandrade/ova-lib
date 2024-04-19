#include "stack.h"
#include "list.h"
#include <stdlib.h>

void linked_stack_push(stack *self, void *item) {
    list *lst = (list *)self->impl;
    lst->insert(lst, item, 0);
}

void *linked_stack_pop(stack *self) {
    list *lst = (list *)self->impl;
    if (lst->size(lst) == 0) return NULL;
    void *item = lst->get(lst, 0);
    lst->remove(lst, 0);
    return item;
}

