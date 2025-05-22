/**
 * @file list.c
 * @brief Factory functions for list instances.
 */

#include "../../include/list.h"
#include "array_list.h"
#include "linked_list.h"

/**
 * @brief Factory function that creates a specific list implementation.
 *
 * @param type Desired list type.
 * @param initial_capacity Initial capacity for array based lists.
 * @return Pointer to the created list or NULL on failure.
 */
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
