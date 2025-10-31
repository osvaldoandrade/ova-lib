#ifndef SORTED_LIST_H
#define SORTED_LIST_H

#include "../../include/types.h"
#include "../../include/list.h"

list *create_sorted_list(int initial_capacity, comparator cmp);

#endif // SORTED_LIST_H
