#include "../../include/list.h"
#include "array_list.h"
#include "linked_list.h"

list *create_list(ListType type, int initial_capacity) {
    switch (type) {
        case ARRAY_LIST:
            return create_array_list(initial_capacity);
        case LINKED_LIST:
            return create_linked_list();
        case SORTED_LIST:
            break;
        default:
            return NULL;
    }
    return NULL;
}
