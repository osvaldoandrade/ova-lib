#include "sorted_list.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    void **items;
    int size;
    int capacity;
    comparator cmp;
} sorted_list_impl;

static ova_error_code sorted_list_insert(list *self, void *item, int index);
static void *sorted_list_get(list *self, int index);
static ova_error_code sorted_list_remove(list *self, int index);
static int sorted_list_size(const list *self);
static void sorted_list_free(list *self);

static int clamp_initial_capacity(int initial_capacity) {
    return initial_capacity > 0 ? initial_capacity : 4;
}

list *create_sorted_list(int initial_capacity, comparator cmp) {
    if (!cmp) {
        return NULL;
    }

    list *lst = malloc(sizeof(list));
    sorted_list_impl *impl = malloc(sizeof(sorted_list_impl));

    if (!lst || !impl) {
        free(lst);
        free(impl);
        return NULL;
    }

    impl->capacity = clamp_initial_capacity(initial_capacity);
    impl->size = 0;
    impl->cmp = cmp;
    impl->items = malloc(sizeof(void *) * (size_t)impl->capacity);

    if (!impl->items) {
        free(impl);
        free(lst);
        return NULL;
    }

    lst->impl = impl;
    lst->insert = sorted_list_insert;
    lst->get = sorted_list_get;
    lst->remove = sorted_list_remove;
    lst->size = sorted_list_size;
    lst->free = sorted_list_free;
    lst->user_data = NULL;

    return lst;
}

static sorted_list_impl *get_impl(const list *self) {
    return self ? (sorted_list_impl *)self->impl : NULL;
}

static int ensure_capacity(sorted_list_impl *impl) {
    if (impl->size < impl->capacity) {
        return 1; // Already has capacity
    }

    int new_capacity = impl->capacity * 2;
    void **new_items = realloc(impl->items, sizeof(void *) * (size_t)new_capacity);
    if (new_items == NULL) {
        return 0; // Signal failure
    }
    impl->items = new_items;
    impl->capacity = new_capacity;
    return 1; // Success
}

static int find_insert_position(sorted_list_impl *impl, void *item) {
    int left = 0;
    int right = impl->size;

    while (left < right) {
        int mid = left + (right - left) / 2;
        if (impl->cmp(item, impl->items[mid]) > 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

static ova_error_code sorted_list_insert(list *self, void *item, int index) {
    (void)index; // Index is ignored to keep the list sorted
    sorted_list_impl *impl = get_impl(self);
    if (!impl) {
        return OVA_ERROR_INVALID_ARG;
    }

    if (!ensure_capacity(impl)) {
        return OVA_ERROR_MEMORY;
    }

    int insert_pos = find_insert_position(impl, item);
    memmove(&impl->items[insert_pos + 1], &impl->items[insert_pos],
            (size_t)(impl->size - insert_pos) * sizeof(void *));
    impl->items[insert_pos] = item;
    impl->size++;
    return OVA_SUCCESS;
}

static void *sorted_list_get(list *self, int index) {
    sorted_list_impl *impl = get_impl(self);
    if (!impl || index < 0 || index >= impl->size) {
        return NULL;
    }
    return impl->items[index];
}

static ova_error_code sorted_list_remove(list *self, int index) {
    sorted_list_impl *impl = get_impl(self);
    if (!impl) {
        return OVA_ERROR_INVALID_ARG;
    }
    if (index < 0 || index >= impl->size) {
        return OVA_ERROR_INDEX_OUT_OF_BOUNDS;
    }

    memmove(&impl->items[index], &impl->items[index + 1],
            (size_t)(impl->size - index - 1) * sizeof(void *));
    impl->size--;
    return OVA_SUCCESS;
}

static int sorted_list_size(const list *self) {
    const sorted_list_impl *impl = get_impl(self);
    return impl ? impl->size : 0;
}

static void sorted_list_free(list *self) {
    sorted_list_impl *impl = get_impl(self);
    if (impl) {
        free(impl->items);
        free(impl);
    }
    free(self);
}
