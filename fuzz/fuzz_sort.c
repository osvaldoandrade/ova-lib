#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/list.h"
#include "../include/sort.h"

#define MAX_VALUES 64

static int int_compare(const void *a, const void *b) {
    int lhs = a ? *(const int *)a : 0;
    int rhs = b ? *(const int *)b : 0;
    return (lhs > rhs) - (lhs < rhs);
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    list *lst = create_list(ARRAY_LIST, 8, int_compare);
    sorter *s = create_sorter(int_compare);
    if (!lst || !s) {
        if (lst) lst->free(lst);
        if (s) s->free(s);
        return 0;
    }

    int values[MAX_VALUES] = {0};
    int value_count = 0;
    size_t offset = 0;

    while (offset + sizeof(int) <= Size && value_count < MAX_VALUES) {
        int v = 0;
        memcpy(&v, Data + offset, sizeof(int));
        offset += sizeof(int);
        values[value_count] = v;
        lst->insert(lst, &values[value_count], value_count);
        value_count++;
    }

    while (offset < Size) {
        uint8_t op = Data[offset++];
        int current_size = lst->size(lst);
        switch (op % 6) {
            case 0:
                if (current_size > 0) {
                    s->sort(s, lst);
                }
                break;
            case 1:
                if (current_size > 1) {
                    s->shuffle(s, lst);
                }
                break;
            case 2:
                if (current_size > 1) {
                    s->reverse(s, lst);
                }
                break;
            case 3:
                if (current_size > 0 && value_count > 0) {
                    int idx = Data[offset % Size] % value_count;
                    s->binary_search(s, lst, &values[idx]);
                }
                break;
            case 4: {
                void *min = NULL;
                void *max = NULL;
                s->min_max(s, lst, &min, &max);
                if (min && max && current_size > 1) {
                    int i = op % current_size;
                    int j = (op / 2) % current_size;
                    s->swap(s, lst, i, j);
                }
                break;
            }
            default:
                if (offset + sizeof(int) <= Size && value_count < MAX_VALUES) {
                    int v = 0;
                    memcpy(&v, Data + offset, sizeof(int));
                    offset += sizeof(int);
                    int pos = current_size > 0 ? (int)(Data[offset - 1] % (current_size + 1)) : 0;
                    values[value_count] = v;
                    lst->insert(lst, &values[value_count], pos);
                    value_count++;
                } else if (current_size > 0) {
                    int pos = op % current_size;
                    lst->remove(lst, pos);
                }
                break;
        }
    }

    s->free(s);
    lst->free(lst);
    return 0;
}
