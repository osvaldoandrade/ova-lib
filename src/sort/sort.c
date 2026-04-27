#include "../../include/sort.h"
#include <stdlib.h>
#include <string.h>

typedef struct sorter_impl {
    comparator cmp;
} sorter_impl;

static sorter_impl *sorter_impl_from_self(const sorter *self) {
    return self ? (sorter_impl *)self->impl : NULL;
}

static void sorter_free(sorter *self) {
    if (!self) {
        return;
    }

    free(self->impl);
    self->impl = NULL;
    free(self);
}

/**
 * @brief Swaps two elements in a list at the specified indices.
 *
 * This function swaps two elements in a list at the given indices.
 * The elements are swapped by removing them from the list and then
 * inserting them back at the opposite indices.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list structure.
 * @param index1 The index of the first element to be swapped.
 * @param index2 The index of the second element to be swapped.
 */
void sorter_swap(sorter *self, list *lst, int index1, int index2) {
    (void)self;
    void *temp1 = lst->get(lst, index1);
    void *temp2 = lst->get(lst, index2);

    lst->remove(lst, index1);
    lst->insert(lst, temp2, index1);
    lst->remove(lst, index2);
    lst->insert(lst, temp1, index2);
}

/**
 * @brief Partitions a list for sorting using a pivot element.
 *
 * This function partitions a list for sorting using a pivot element. It rearranges the items in the list
 * such that all the items that are less than or equal to the pivot element are moved to the left of the pivot,
 * and all the items that are greater than the pivot are moved to the right of the pivot. The pivot element is
 * chosen as the last element in the list.
 *
 * @param lst A pointer to the list structure.
 * @param low The starting index of the partition.
 * @param high The ending index of the partition.
 * @param self A pointer to the sorter structure.
 * @return The index of the pivot element after partitioning.
 */
static int sorter_partition(list *lst, int low, int high, sorter *self) {
    sorter_impl *impl = sorter_impl_from_self(self);
    void *pivot = lst->get(lst, high);
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (impl && impl->cmp(lst->get(lst, j), pivot) <= 0) {
            i++;
            self->swap(self, lst, i, j);
        }
    }
    self->swap(self, lst, i + 1, high);
    return i + 1;
}

/**
 * @brief Sorts a list using the quicksort algorithm.
 *
 * This function sorts a list using the quicksort algorithm.
 * The list is sorted in place, i.e., the original content of the list is modified.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list to be sorted.
 */
static void sorter_quick(sorter *self, list *lst) {
    int size = lst->size(lst);
    if (size < 2) {
        return;
    }

    int *stack = malloc(sizeof(int) * (size_t)size * 2);
    if (!stack) return;

    int top = 0;
    stack[top++] = 0;
    stack[top++] = size - 1;

    while (top > 0) {
        int high = stack[--top];
        int low = stack[--top];
        int pi = sorter_partition(lst, low, high, self);

        if (pi - 1 > low) {
            stack[top++] = low;
            stack[top++] = pi - 1;
        }
        if (pi + 1 < high) {
            stack[top++] = pi + 1;
            stack[top++] = high;
        }
    }
    free(stack);
}

/**
 * @brief Merges two sorted sub-arrays within a temporary buffer.
 *
 * @param arr  Array of payload pointers.
 * @param tmp  Temporary buffer for merging.
 * @param low  Start index of the first half.
 * @param mid  End index of the first half (exclusive start of second half).
 * @param high End index of the range (exclusive).
 * @param cmp  Comparator function.
 */
static void merge(void **arr, void **tmp, int low, int mid, int high,
                  comparator cmp) {
    memcpy(tmp + low, arr + low, (size_t)(high - low) * sizeof(void *));

    int i = low;
    int j = mid;
    int k = low;

    while (i < mid && j < high) {
        if (cmp(tmp[i], tmp[j]) <= 0) {
            arr[k++] = tmp[i++];
        } else {
            arr[k++] = tmp[j++];
        }
    }
    while (i < mid) {
        arr[k++] = tmp[i++];
    }
    while (j < high) {
        arr[k++] = tmp[j++];
    }
}

/**
 * @brief Recursively sorts a sub-array using cache-oblivious merge sort.
 *
 * The recursive halving naturally creates sub-problems that fit into each
 * level of the cache hierarchy without requiring any tuning parameters.
 *
 * @param arr  Array of payload pointers.
 * @param tmp  Temporary buffer for merging.
 * @param low  Start index (inclusive).
 * @param high End index (exclusive).
 * @param cmp  Comparator function.
 */
static void merge_sort_recursive(void **arr, void **tmp, int low, int high,
                                 comparator cmp) {
    if (high - low < 2) {
        return;
    }
    int mid = low + (high - low) / 2;
    merge_sort_recursive(arr, tmp, low, mid, cmp);
    merge_sort_recursive(arr, tmp, mid, high, cmp);
    merge(arr, tmp, low, mid, high, cmp);
}

/**
 * @brief Sorts a list using cache-oblivious merge sort.
 *
 * Elements are copied into a contiguous array for cache-friendly access,
 * sorted using a recursive merge sort (which naturally adapts to all cache
 * levels), and then written back to the list.  This guarantees O(n log n)
 * worst-case time complexity.
 *
 * @param self A pointer to the sorter structure.
 * @param lst  A pointer to the list to be sorted.
 */
static void sorter_merge(sorter *self, list *lst) {
    sorter_impl *impl = sorter_impl_from_self(self);
    if (!impl) return;

    int size = lst->size(lst);
    if (size < 2) {
        return;
    }

    void **arr = malloc((size_t)size * sizeof(void *));
    if (!arr) return;

    void **tmp = malloc((size_t)size * sizeof(void *));
    if (!tmp) {
        free(arr);
        return;
    }

    for (int i = 0; i < size; i++) {
        arr[i] = lst->get(lst, i);
    }

    merge_sort_recursive(arr, tmp, 0, size, impl->cmp);

    for (int i = 0; i < size; i++) {
        lst->remove(lst, i);
        lst->insert(lst, arr[i], i);
    }

    free(tmp);
    free(arr);
}

/**
 * @brief Shuffle the elements in a list using the Fisher-Yates algorithm.
 *
 * This function shuffles the elements in a list using the Fisher-Yates algorithm.
 * The Fisher-Yates algorithm randomly swaps each element in the list with another element,
 * resulting in a randomized order of elements.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list to be shuffled.
 */
void collections_shuffle(sorter *self, list *lst) {
    int n = lst->size(lst);
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        self->swap(self, lst, i, j);
    }
}

/**
 * @brief Reverse the elements in a list.
 *
 * This function reverses the elements in a list using the specified sorter instance.
 * It swaps pairs of elements starting from the first and last positions, until the first and last indices meet in the middle.
 *
 * @param self A pointer to the sorter instance.
 * @param lst The list to be reversed.
 */
void collections_reverse(sorter *self, list *lst) {
    int i = 0;
    int j = lst->size(lst) - 1;
    while (i < j) {
        self->swap(self, lst, i, j);
        i++;
        j--;
    }
}

/**
 * @brief Performs binary search on a sorted list to find the index of a specific item.
 *
 * This function performs a binary search on a sorted list to find the index of a specific item.
 * It uses the sorter implementation to compare the item with the items in the list.
 * If the item is found, the function returns the index of the item.
 * If the item is not found, the function returns -1.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the sorted list.
 * @param item A pointer to the item to be searched.
 * @return The index of the item in the list, or -1 if the item is not found.
 */
int collections_binary_search(sorter *self, list *lst, void *item) {
    sorter_impl *impl = sorter_impl_from_self(self);
    int low = 0;
    int high = lst->size(lst) - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        void *mid_item = lst->get(lst, mid);
        int cmp_result = impl ? impl->cmp(mid_item, item) : 0;

        if (cmp_result == 0) {
            return mid;
        } else if (cmp_result < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

/**
 * @brief Copies the items from one list to another.
 *
 * This function is used to copy the items from one list to another.
 * It iterates over the source list and inserts each item into the destination list at the same index.
 *
 * @param self A pointer to the sorter structure.
 * @param src A pointer to the source list.
 * @param dest A pointer to the destination list.
 */
void collections_copy(sorter *self, list *src, list *dest) {
    (void)self;
    int size = src->size(src);
    for (int i = 0; i < size; i++) {
        void *item = src->get(src, i);
        dest->insert(dest, item, i);
    }
}

/**
 * @brief Finds the minimum and maximum elements in a list using a custom sorter.
 *
 * This function finds the minimum and maximum elements in a given list using a custom sorter.
 * The minimum and maximum elements are returned through the `min` and `max` parameters, respectively.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list containing the elements.
 * @param min A pointer to a pointer where the minimum element will be stored.
 * @param max A pointer to a pointer where the maximum element will be stored.
 */
void collections_min_max(sorter *self, list *lst, void **min, void **max) {
    sorter_impl *impl = sorter_impl_from_self(self);
    int size = lst->size(lst);
    if (size == 0) {
        *min = NULL;
        *max = NULL;
        return;
    }

    *min = lst->get(lst, 0);
    *max = lst->get(lst, 0);

    int i = (size % 2 == 0) ? 1 : 2;

    for (; i < size - 1; i += 2) {
        void *first = lst->get(lst, i);
        void *second = lst->get(lst, i + 1);

        if (impl && impl->cmp(first, second) > 0) {
            if (impl->cmp(first, *max) > 0) *max = first;
            if (impl->cmp(second, *min) < 0) *min = second;
        } else {
            if (impl && impl->cmp(second, *max) > 0) *max = second;
            if (impl && impl->cmp(first, *min) < 0) *min = first;
        }
    }

    if (size % 2 != 0) {
        void *last = lst->get(lst, size - 1);
        if (impl && impl->cmp(last, *max) > 0) *max = last;
        if (impl && impl->cmp(last, *min) < 0) *min = last;
    }
}


/**
 * @brief Find the maximum element in a list.
 *
 * This function iterates through a given list and compares each element using a custom sorter.
 * It returns the maximum element found using comparison function provided.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list structure.
 * @return A pointer to the maximum element found in the list, or NULL if the list is empty.
 */
void *collections_max(sorter *self, list *lst) {
    sorter_impl *impl = sorter_impl_from_self(self);
    int size = lst->size(lst);
    if (size == 0) return NULL; // Lista vazia, retorna nulo

    void *maximum = lst->get(lst, 0); // Inicia com o primeiro elemento

    for (int i = 1; i < size; i++) {
        void *current = lst->get(lst, i);
        if (impl && impl->cmp(current, maximum) > 0) {
            maximum = current; // Encontra novo máximo
        }
    }

    return maximum;
}

/**
 * @brief Find the minimum element in a list.
 *
 * This function iterates through a given list and compares each element using a custom sorter.
 * It returns the minimum element found using comparison function provided.
 *
 * @param self A pointer to the `sorter` struct which contains the comparison function.
 * @param lst A pointer to the `list` struct where the minimum element needs to be found.
 * @return A pointer to the minimum element found in the list, or NULL if the list is empty.
 */
void *collections_min(sorter *self, list *lst) {
    sorter_impl *impl = sorter_impl_from_self(self);
    int size = lst->size(lst);
    if (size == 0) return NULL; // Lista vazia, retorna nulo

    void *minimum = lst->get(lst, 0); // Inicia com o primeiro elemento

    for (int i = 1; i < size; i++) {
        void *current = lst->get(lst, i);
        if (impl && impl->cmp(current, minimum) < 0) {
            minimum = current; // Encontra novo mínimo
        }
    }

    return minimum;
}

/**
 * @brief Creates a sorter structure.
 *
 * This function creates and allocates memory for a sorter structure.
 * It initializes the function pointers and assigns the comparator function.
 *
 * @param data A pointer to the list of data to be sorted.
 * @param cmp The comparator function for sorting the data.
 * @return A pointer to the created sorter structure.
 * @retval NULL If memory allocation failed or if the sorter creation was unsuccessful.
 */
sorter *create_sorter(comparator cmp) {
    sorter *s = malloc(sizeof(sorter));
    if (!s) return NULL;

    sorter_impl *impl = malloc(sizeof(sorter_impl));
    if (!impl) {
        free(s);
        return NULL;
    }

    impl->cmp = cmp;

    s->impl = impl;
    s->sort = sorter_quick;
    s->swap = sorter_swap;
    s->shuffle = collections_shuffle;
    s->reverse = collections_reverse;
    s->binary_search = collections_binary_search;
    s->copy = collections_copy;
    s->min = collections_min;
    s->max = collections_max;
    s->min_max = collections_min_max;
    s->free = sorter_free;

    return s;
}

/**
 * @brief Creates a sorter that uses cache-oblivious merge sort.
 *
 * Identical to create_sorter() but the sort function pointer uses a
 * cache-oblivious merge sort instead of quicksort.  This provides
 * O(n log n) worst-case time and better cache utilization without any
 * tuning parameters.
 *
 * @param cmp The comparator function for sorting the data.
 * @return A pointer to the created sorter structure.
 * @retval NULL If memory allocation failed.
 */
sorter *create_merge_sorter(comparator cmp) {
    sorter *s = create_sorter(cmp);
    if (!s) return NULL;

    s->sort = sorter_merge;
    return s;
}
