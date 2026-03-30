#include "../../include/list.h"
#include "array_list.h"
#include "linked_list.h"
#include "sorted_list.h"

static void list_insert_bulk_impl(list *self, void **elements, int count) {
    if (!self || !elements || count <= 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        self->insert(self, elements[i], self->size(self));
    }
}

list *create_list(ListType type, int initial_capacity, comparator cmp) {
    list *out = NULL;

    switch (type) {
        case ARRAY_LIST:
            out = create_array_list(initial_capacity);
            break;
        case LINKED_LIST:
            out = create_linked_list();
            break;
        case SORTED_LIST:
            out = create_sorted_list(initial_capacity, cmp);
            break;
        default:
            return NULL;
    }

    if (out) {
        out->insert_bulk = list_insert_bulk_impl;
    }
    return out;
}
