#include "../../include/list.h"
#include "array_list.h"
#include "linked_list.h"
#include "sorted_list.h"

list *create_list(ListType type, int initial_capacity, comparator cmp) {
    switch (type) {
        case ARRAY_LIST:
            return create_array_list(initial_capacity);
        case LINKED_LIST:
            return create_linked_list();
        case SORTED_LIST:
            return create_sorted_list(initial_capacity, cmp);
        default:
            return NULL;
    }
    return NULL;
}

void list_insert_bulk(list *l, void **elements, int count) {
    if (!l || !elements || count <= 0) return;
    for (int i = 0; i < count; i++) {
        l->insert(l, elements[i], l->size(l));
    }
}
