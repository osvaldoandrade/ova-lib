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
} linked_list_impl;

static void linked_list_insert(list *self, void *item, int index);
static void *linked_list_get(list *self, int index);
static void linked_list_remove(list *self, int index);
static void linked_list_free(list *self);
static int linked_list_size(const list *self);

list *create_linked_list() {
    list *lst = malloc(sizeof(list));
    linked_list_impl *impl = malloc(sizeof(linked_list_impl));

    if (lst && impl) {
        impl->head = NULL;
        impl->tail = NULL;
        impl->size = 0;

        lst->impl = impl;
        lst->insert = linked_list_insert;
        lst->get = linked_list_get;
        lst->remove = linked_list_remove;
        lst->size = linked_list_size;
        lst->free = linked_list_free;
        return lst;
    }

    free(lst);
    free(impl);
    return NULL;
}

static void linked_list_insert(list *self, void *item, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    linked_list_node *new_node = malloc(sizeof(linked_list_node));
    if (!new_node) return;

    new_node->data = item;
    new_node->next = NULL;
    new_node->prev = NULL;

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
            if ((index < impl->size / 2 && current_index == index) ||
                (index >= impl->size / 2 && current_index == index)) {
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
}

static void *linked_list_get(list *self, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    linked_list_node *current = impl->head;
    for (int i = 0; i < index && current; i++) {
        current = current->next;
    }
    return current ? current->data : NULL;
}

static void linked_list_remove(list *self, int index) {
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    linked_list_node *current = impl->head;
    for (int i = 0; i < index && current; i++) {
        current = current->next;
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
}

static int linked_list_size(const list *self) {
    if (!self || !self->impl) return 0;
    linked_list_impl *impl = (linked_list_impl *)self->impl;
    return impl->size;
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
