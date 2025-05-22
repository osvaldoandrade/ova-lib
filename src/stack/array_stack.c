/**
 * @file array_stack.c
 * @brief Array backed stack helpers.
 */

#include "stack.h"
#include "list.h"
#include <stdlib.h>

/**
 * @brief Push an item onto the top of the array-backed stack.
 */
void array_stack_push(stack *self, void *item) {
    list *lst = (list *)self->impl;
    lst->insert(lst, item, lst->size(lst));
}

/**
 * @brief Pop the item from the top of the array-backed stack.
 */
void *array_stack_pop(stack *self) {
    list *lst = (list *)self->impl;
    if (lst->size(lst) == 0) return NULL;
    void *item = lst->get(lst, lst->size(lst) - 1);
    lst->remove(lst, lst->size(lst) - 1);
    return item;
}


