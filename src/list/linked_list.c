#include "../../include/list.h"
#include <stdlib.h>

typedef struct linked_list_node {
    void *data;
    struct linked_list_node *next;
    struct linked_list_node *prev;
} linked_list_node;

typedef struct {
    linked_list_node *head;
    linked_list_node *tail;
    int size;
    /* Cursor cache to make sequential get(i), get(i+1), ... amortized O(1).
     * cursor_index == -1 means cache is invalid. */
    linked_list_node *cursor_node;
    int cursor_index;
} linked_list_impl;

static ova_error_code linked_list_insert(list *self, void *item, int index);
static void *linked_list_get(list *self, int index);
static ova_error_code linked_list_remove(list *self, int index);
static void linked_list_clear(list *self);
static void linked_list_free(list *self);
static int linked_list_size(const list *self);

list *create_linked_list(void) {
    list *lst = malloc(sizeof(list));
    linked_list_impl *impl = malloc(sizeof(linked_list_impl));

    if (lst && impl) {
        impl->head = NULL;
        impl->tail = NULL;
        impl->size = 0;
        impl->cursor_node = NULL;
        impl->cursor_index = -1;

        lst->impl = impl;
        lst->insert = linked_list_insert;
        lst->get = linked_list_get;
        lst->remove = linked_list_remove;
        lst->size = linked_list_size;
        lst->clear = linked_list_clear;
        lst->free = linked_list_free;
        lst->user_data = NULL;
        return lst;
    }

    free(lst);
    free(impl);
    return NULL;
}

static ova_error_code linked_list_insert(list *self, void *item, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    if (index < 0 || index > impl->size) return OVA_ERROR_INDEX_OUT_OF_BOUNDS;
    linked_list_node *new_node = malloc(sizeof(linked_list_node));
    if (!new_node) return OVA_ERROR_MEMORY;

    new_node->data = item;
    new_node->next = NULL;
    new_node->prev = NULL;

    impl->cursor_node = NULL;
    impl->cursor_index = -1;

    if (index == 0) {
        if (impl->head == NULL) {
            impl->head = impl->tail = new_node;
        } else {
            new_node->next = impl->head;
            impl->head->prev = new_node;
            impl->head = new_node;
        }
    } else if (index == impl->size) {
        new_node->prev = impl->tail;
        if (impl->tail) {
            impl->tail->next = new_node;
        }
        impl->tail = new_node;
    } else {
        linked_list_node *current = (index < impl->size / 2) ? impl->head : impl->tail;
        int current_index = (index < impl->size / 2) ? 0 : impl->size - 1;

        while (current) {
            if (current_index == index) {
                new_node->next = current;
                new_node->prev = current->prev;
                if (current->prev) {
                    current->prev->next = new_node;
                }
                current->prev = new_node;

                if (index == 0) impl->head = new_node;
                if (index == impl->size) impl->tail = new_node;
                break;
            }
            current = (index < impl->size / 2) ? current->next : current->prev;
            current_index = (index < impl->size / 2) ? current_index + 1 : current_index - 1;
        }
    }
    impl->size++;
    return OVA_SUCCESS;
}

static void *linked_list_get(list *self, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    if (index < 0 || index >= impl->size) return NULL;

    linked_list_node *current = NULL;
    int start_index = 0;
    int dist_head = index;
    int dist_tail = impl->size - 1 - index;
    int best = dist_head < dist_tail ? dist_head : dist_tail;

    if (impl->cursor_node && impl->cursor_index >= 0) {
        int dist_cursor_abs = index - impl->cursor_index;
        if (dist_cursor_abs < 0) dist_cursor_abs = -dist_cursor_abs;
        if (dist_cursor_abs < best) {
            current = impl->cursor_node;
            start_index = impl->cursor_index;
            best = dist_cursor_abs;
        }
    }

    if (!current) {
        if (dist_head <= dist_tail) {
            current = impl->head;
            start_index = 0;
        } else {
            current = impl->tail;
            start_index = impl->size - 1;
        }
    }

    while (current && start_index < index) {
        current = current->next;
        start_index++;
    }
    while (current && start_index > index) {
        current = current->prev;
        start_index--;
    }

    if (current) {
        impl->cursor_node = current;
        impl->cursor_index = index;
        return current->data;
    }
    return NULL;
}

static ova_error_code linked_list_remove(list *self, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    if (index < 0 || index >= impl->size) return OVA_ERROR_INDEX_OUT_OF_BOUNDS;
    impl->cursor_node = NULL;
    impl->cursor_index = -1;
    linked_list_node *current;
    if (index < impl->size / 2) {
        current = impl->head;
        for (int i = 0; i < index && current; i++) {
            current = current->next;
        }
    } else {
        current = impl->tail;
        for (int i = impl->size - 1; i > index && current; i--) {
            current = current->prev;
        }
    }
    if (current) {
        if (current->prev) {
            current->prev->next = current->next;
        } else {
            impl->head = current->next;
        }
        if (current->next) {
            current->next->prev = current->prev;
        } else {
            impl->tail = current->prev;
        }
        free(current);
        impl->size--;
    }
    return OVA_SUCCESS;
}

static int linked_list_size(const list *self) {
    if (!self || !self->impl) return 0;
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    return impl->size;
}

static void linked_list_clear(list *self) {
    if (self && self->impl) {
        linked_list_impl *impl = (linked_list_impl *)self->impl;
        linked_list_node *current = impl->head;
        while (current) {
            linked_list_node *next = current->next;
            free(current);
            current = next;
        }
        impl->head = NULL;
        impl->tail = NULL;
        impl->size = 0;
        impl->cursor_node = NULL;
        impl->cursor_index = -1;
    }
}

static void linked_list_free(list *self) {
    if (self->impl) {
        linked_list_impl *impl = (linked_list_impl *)self->impl;
        linked_list_node *current = impl->head;
        while (current) {
            linked_list_node *next = current->next;
            free(current);
            current = next;
        }
        free(impl);
        self->impl = NULL;
    }
    free(self);
}
