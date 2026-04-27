#include "../../include/list.h"
#include "../utils/capacity_utils.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    void **items;
    int size;
    int capacity;
} array_list_impl;

static size_t active_item_buffers = 0;

static ova_error_code array_list_insert(list *self, void *item, int index);

static void *array_list_get(list *self, int index);

static ova_error_code array_list_remove(list *self, int index);

static int array_list_size(const list *self);

static void array_list_clear(list *self);

static void array_list_free(list *self);

list *create_array_list(int initial_capacity) {
    list *lst = malloc(sizeof(list));
    array_list_impl *impl = malloc(sizeof(array_list_impl));

    if (lst && impl && (impl->items = malloc((size_t)initial_capacity * sizeof(void *)))) {
        impl->capacity = initial_capacity;
        impl->size = 0;
        active_item_buffers++;

        lst->impl = impl;
        lst->insert = array_list_insert;
        lst->get = array_list_get;
        lst->remove = array_list_remove;
        lst->size = array_list_size;
        lst->clear = array_list_clear;
        lst->free = array_list_free;
        lst->user_data = NULL;
        return lst;
    }

    free(lst);
    free(impl);
    return NULL;
}

static int get_size_from_impl(void *impl) {
    array_list_impl *internal = (array_list_impl *) impl;
    return internal->size;
}

static int ensure_capacity(array_list_impl *impl) {
    if (impl->size >= impl->capacity) {
        int new_capacity = safe_double_capacity(impl->capacity);
        if (new_capacity == impl->capacity) {
            return 0; // Already at maximum capacity
        }
        void **new_items = realloc(impl->items, (size_t)new_capacity * sizeof(void *));
        if (new_items == NULL) {
            return 0; // Signal failure
        }
        impl->items = new_items;
        impl->capacity = new_capacity;
    }
    return 1; // Success
}

static ova_error_code array_list_insert(list *self, void *item, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    if (index < 0 || index > impl->size) return OVA_ERROR_INDEX_OUT_OF_BOUNDS;
    if (!ensure_capacity(impl)) {
        return OVA_ERROR_MEMORY;
    }
    memmove(&impl->items[index + 1], &impl->items[index], (size_t)(impl->size - index) * sizeof(void *));
    impl->items[index] = item;
    impl->size++;
    return OVA_SUCCESS;
}

static void *array_list_get(list *self, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    if (index < 0 || index >= impl->size) return NULL;
    return impl->items[index];
}

static ova_error_code array_list_remove(list *self, int index) {
    array_list_impl *impl = (array_list_impl *) self->impl;
    if (index < 0 || index >= impl->size) return OVA_ERROR_INDEX_OUT_OF_BOUNDS;
    memmove(&impl->items[index], &impl->items[index + 1], (size_t)(impl->size - index - 1) * sizeof(void *));
    impl->size--;
    return OVA_SUCCESS;
}

static int array_list_size(const list *self) {
    if (!self || !self->impl) return 0;
    return get_size_from_impl(self->impl);
}

static void array_list_clear(list *self) {
    if (self && self->impl) {
        array_list_impl *impl = (array_list_impl *) self->impl;
        impl->size = 0;
    }
}

static void array_list_free(list *self) {
    if (self) {
        array_list_impl *impl = (array_list_impl *) self->impl;
        if (impl) {
            if (impl->items) {
                free(impl->items);
                impl->items = NULL;
                if (active_item_buffers > 0) {
                    active_item_buffers--;
                }
            }
            free(impl);
            self->impl = NULL;
        }
        free(self);
    }
}

size_t array_list_active_buffer_count(void) {
    return active_item_buffers;
}
