#ifndef SORT_H
#define SORT_H

#include "list.h"

typedef struct sorter sorter;

struct sorter {
    comparator cmp;
    void (*sort)(sorter* self, list* lst);
    void (*shuffle)(sorter* self, list* lst);
    void (*reverse)(sorter* self, list* lst);
    int (*binary_search)(sorter* self, list* lst, void* item);
    void (*swap)(sorter* self, list* lst, int i, int j);
    void *(*min)(sorter* self, list* lst);
    void *(*max)(sorter* self, list* lst);
    void (*copy)(sorter* self, list* src, list* dest);
};

sorter* create_sorter(list *data, comparator cmp);

#endif // SORT_H
