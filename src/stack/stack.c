#include "../../include/stack.h"
#include "../../include/list.h"
#include "linked_stack.h"
#include "array_stack.h"
#include <stdlib.h>

static void stack_free(stack *self);
static void stack_clear(stack *self);
static void *stack_top(const stack *self);
static int stack_is_empty(const stack *self);
static int stack_size(const stack *self);
static stack *stack_clone_shallow(const stack *self);
static stack *stack_clone_deep(const stack *self, element_copier copier);

stack *create_stack(StackType type) {
    stack *stk = malloc(sizeof(stack));
    if (!stk) return NULL;

    if (type == ARRAY_STACK) {
        stk->impl = create_list(ARRAY_LIST, 10, NULL);
        stk->push = array_stack_push;
        stk->pop = array_stack_pop;
    } else if (type == LINKED_STACK) {
        stk->impl = create_list(LINKED_LIST, 10, NULL);
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
    stk->clear = stack_clear;
    stk->free = stack_free;
    stk->clone_shallow = stack_clone_shallow;
    stk->clone_deep = stack_clone_deep;
    stk->user_data = NULL;

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

static void stack_clear(stack *self) {
    if (!self || !self->impl) return;
    list *lst = (list *)self->impl;
    lst->clear(lst);
}

void stack_free(stack *self) {
    if (!self) return;
    if (self->impl) {
        list *lst = (list *) self->impl;
        lst->free(lst);
    }
    free(self);
}

static stack *stack_clone_shallow(const stack *self) {
    if (!self) {
        return NULL;
    }

    list *lst = (list *)self->impl;
    if (!lst) {
        return NULL;
    }

    StackType type = (self->push == array_stack_push) ? ARRAY_STACK : LINKED_STACK;
    stack *copy = create_stack(type);
    if (!copy) {
        return NULL;
    }

    /* Insert elements in internal list order so the clone mirrors the
       original stack's internal structure. */
    int n = lst->size(lst);
    list *copy_lst = (list *)copy->impl;
    for (int i = 0; i < n; i++) {
        void *elem = lst->get(lst, i);
        ova_error_code err = copy_lst->insert(copy_lst, elem, copy_lst->size(copy_lst));
        if (err != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
    }
    copy->user_data = self->user_data;
    return copy;
}

static stack *stack_clone_deep(const stack *self, element_copier copier) {
    if (!self || !copier) {
        return NULL;
    }

    list *lst = (list *)self->impl;
    if (!lst) {
        return NULL;
    }

    StackType type = (self->push == array_stack_push) ? ARRAY_STACK : LINKED_STACK;
    stack *copy = create_stack(type);
    if (!copy) {
        return NULL;
    }

    int n = lst->size(lst);
    list *copy_lst = (list *)copy->impl;
    for (int i = 0; i < n; i++) {
        void *elem = lst->get(lst, i);
        void *dup = copier(elem);
        if (!dup) {
            copy->free(copy);
            return NULL;
        }
        ova_error_code err = copy_lst->insert(copy_lst, dup, copy_lst->size(copy_lst));
        if (err != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
    }
    copy->user_data = self->user_data;
    return copy;
}