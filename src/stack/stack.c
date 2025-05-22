#include "../../include/stack.h"
#include "../../include/list.h"
#include "linked_stack.h"
#include "array_stack.h"
#include <stdlib.h>

static void stack_free(stack *self);
static void *stack_top(const stack *self);
static int stack_is_empty(const stack *self);
static int stack_size(const stack *self);

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

    if (!stk->impl) {
        free(stk);
        return NULL;
    }

    stk->top = stack_top;
    stk->is_empty = stack_is_empty;
    stk->size = stack_size;
    stk->free = stack_free;

    return stk;
}

static void *stack_top(const stack *self) {
    list *lst = (list *)self->impl;
    if (!lst || lst->size(lst) == 0) return NULL;
    return lst->get(lst, (self->push == array_stack_push) ? lst->size(lst) - 1 : 0);
}

static int stack_is_empty(const stack *self) {
    list *lst = (list *)self->impl;
    return lst ? (lst->size(lst) == 0) : 1;
}

static int stack_size(const stack *self) {
    list *lst = (list *)self->impl;
    return lst ? lst->size(lst) : 0;
}

void stack_free(stack *self) {
    if (!self) return;
    if (self->impl) {
        list *lst = (list *) self->impl;
        lst->free(lst);
    }
    free(self);
}
