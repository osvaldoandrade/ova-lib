#include "../../include/list.h"
#include "array_list.h"
#include "linked_list.h"
#include "sorted_list.h"

#include <string.h>

static ova_error_code list_insert_bulk_impl(list *self, void **elements, int count) {
    if (!self || !elements || count <= 0) {
        return OVA_ERROR_INVALID_ARG;
    }

    for (int i = 0; i < count; i++) {
        ova_error_code err = self->insert(self, elements[i], self->size(self));
        if (err != OVA_SUCCESS) {
            return err;
        }
    }
    return OVA_SUCCESS;
}

static list *list_clone_shallow_impl(const list *self);
static list *list_clone_deep_impl(const list *self, element_copier copier);

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
        out->_type = type;
        out->_cmp = cmp;
        out->insert_bulk = list_insert_bulk_impl;
        out->clone_shallow = list_clone_shallow_impl;
        out->clone_deep = list_clone_deep_impl;
    }
    return out;
}

static list *list_clone_shallow_impl(const list *self) {
    if (!self) {
        return NULL;
    }

    int n = self->size(self);
    list *copy = create_list(self->_type, n > 0 ? n : 1, self->_cmp);
    if (!copy) {
        return NULL;
    }

    list *mutable_self;
    memcpy(&mutable_self, &self, sizeof(self));

    for (int i = 0; i < n; i++) {
        void *elem = mutable_self->get(mutable_self, i);
        ova_error_code err = copy->insert(copy, elem, copy->size(copy));
        if (err != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
    }
    copy->user_data = self->user_data;
    return copy;
}

static list *list_clone_deep_impl(const list *self, element_copier copier) {
    if (!self || !copier) {
        return NULL;
    }

    int n = self->size(self);
    list *copy = create_list(self->_type, n > 0 ? n : 1, self->_cmp);
    if (!copy) {
        return NULL;
    }

    list *mutable_self;
    memcpy(&mutable_self, &self, sizeof(self));

    for (int i = 0; i < n; i++) {
        void *elem = mutable_self->get(mutable_self, i);
        void *dup = copier(elem);
        if (!dup) {
            copy->free(copy);
            return NULL;
        }
        ova_error_code err = copy->insert(copy, dup, copy->size(copy));
        if (err != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
    }
    copy->user_data = self->user_data;
    return copy;
}
