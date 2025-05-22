/**
 * @file array_list.c
 * @brief Array-based list implementation.
 */

#include "../../include/list.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    void **items;
    int size;
    int capacity;
} array_list_impl;

static void array_list_insert(list *self, void *item, int index);

static void *array_list_get(list *self, int index);

static void array_list_remove(list *self, int index);

static int array_list_size(const list *self);

static void array_list_free(list *self);

/**
 * @brief Create a list backed by a dynamic array.
 *
 * @param initial_capacity Starting capacity of the list.
 * @return Pointer to the new list or NULL on failure.
 */
list *create_array_list(int initial_capacity) {
    list *lst = malloc(sizeof(list));
    array_list_impl *impl = malloc(sizeof(array_list_impl));

    if (lst && impl && (impl->items = malloc(initial_capacity * sizeof(void *)))) {
        impl->capacity = initial_capacity;
        impl->size = 0;

        lst->impl = impl;
        lst->insert = array_list_insert;
        lst->get = array_list_get;
        lst->remove = array_list_remove;
        lst->size = array_list_size;
        lst->free = array_list_free;
        return lst;
    }

    free(lst);
    free(impl);
    return NULL;
}

/**
 * @brief Helper to retrieve the current size from the internal implementation.
 */
int get_size_from_impl(void *impl) {
    array_list_impl *internal = (array_list_impl *) impl;
    return internal->size;
}

/**
 * @brief Grow the internal array if needed.
 */
static void ensure_capacity(array_list_impl *impl) {
    if (impl->size >= impl->capacity) {
        int new_capacity = impl->capacity * 2;
        void **new_items = realloc(impl->items, new_capacity * sizeof(void *));
        if (new_items) {
            impl->items = new_items;
            impl->capacity = new_capacity;
        }
    }
}

/**
 * @brief Insert an element at the specified index.
 */
static void array_list_insert(list *self, void *item, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    ensure_capacity(impl);
    if (index < 0 || index > impl->size) return;
    memmove(&impl->items[index + 1], &impl->items[index], (impl->size - index) * sizeof(void *));
    impl->items[index] = item;
    impl->size++;
}

/**
 * @brief Retrieve the element at a given index.
 */
static void *array_list_get(list *self, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    if (index < 0 || index >= impl->size) return NULL;
    return impl->items[index];
}

/**
 * @brief Remove the element at the specified index.
 */
static void array_list_remove(list *self, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    if (index < 0 || index >= impl->size) return;
    memmove(&impl->items[index], &impl->items[index + 1], (impl->size - index - 1) * sizeof(void *));
    impl->size--;
}

/**
 * @brief Return the number of elements in the list.
 */
static int array_list_size(const list *self) {
    if (!self || !self->impl) return 0;
    return get_size_from_impl(self->impl);
}

/**
 * @brief Destroy the array list and free memory.
 */
static void array_list_free(list *self) {
    if (self->impl) {
        free(self->impl);
        self->impl = NULL;
    }
    free(self);
}
