/**
 * @file stack.c
 * @brief Stack creation and helpers.
 */

#include "../../include/stack.h"
#include "../../include/list.h"
#include "linked_stack.h"
#include "array_stack.h"
#include <stdlib.h>

void stack_free(stack *self);

/**
 * @brief Create a stack of the specified implementation type.
 */
stack *create_stack(StackType type) {
    stack *stk = malloc(sizeof(stack));
    if (!stk) return NULL;

    if (type == ARRAY_STACK) {
        stk->impl = create_list(ARRAY_LIST, 10);
        stk->push = array_stack_push;
        stk->pop = array_stack_pop;
    } else if (type == LINKED_STACK) {
        stk->impl = create_list(LINKED_LIST, 10);
        stk->push = linked_stack_push;
        stk->pop = linked_stack_pop;
    } else {
        free(stk);
        return NULL;
    }

    stk->free = stack_free;

    return stk;
}

/**
 * @brief Free the stack and its underlying list.
 */
void stack_free(stack *self) {
    list *lst = (list *) self->impl;
    lst->free(lst);
    free(self);
}
