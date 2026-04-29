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
 * @brief Snapshot a list into a freshly allocated pointer array.
 *
 * Returns NULL on allocation failure or empty list. Caller must free.
 */
static void **list_snapshot(list *lst, int *out_size) {
    int n = lst->size(lst);
    *out_size = n;
    if (n <= 0) return NULL;
    void **arr = malloc((size_t)n * sizeof(void *));
    if (!arr) return NULL;
    for (int i = 0; i < n; i++) {
        arr[i] = lst->get(lst, i);
    }
    return arr;
}

/**
 * @brief Replace the contents of @p lst with @p arr in O(n).
 *
 * Both array_list and linked_list provide O(1) append at the tail,
 * so a clear-and-bulk-append is linear in the number of elements.
 */
static void list_replace_with(list *lst, void **arr, int n) {
    lst->clear(lst);
    for (int i = 0; i < n; i++) {
        lst->insert(lst, arr[i], i);
    }
}

/**
 * @brief Swaps two elements in a list at the specified indices.
 *
 * Public swap helper preserved for backwards compatibility. It dispatches
 * through the list's own get/remove/insert API and therefore costs O(n)
 * for array-backed lists. Internal sort/shuffle/reverse routines avoid it
 * by operating on a snapshot pointer buffer with true O(1) swaps.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list structure.
 * @param index1 The index of the first element to be swapped.
 * @param index2 The index of the second element to be swapped.
 */
void sorter_swap(sorter *self, list *lst, int index1, int index2) {
    (void)self;
    if (index1 == index2) return;
    void *temp1 = lst->get(lst, index1);
    void *temp2 = lst->get(lst, index2);

    lst->remove(lst, index1);
    lst->insert(lst, temp2, index1);
    lst->remove(lst, index2);
    lst->insert(lst, temp1, index2);
}

static inline void buffer_swap(void **arr, int i, int j) {
    void *t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

/**
 * @brief Partitions a contiguous pointer buffer for quicksort.
 *
 * Uses Lomuto partitioning with O(1) swaps over the raw buffer instead of
 * dispatching through the list interface. This keeps quicksort at the
 * advertised O(n log n) average even for array-backed lists, where the
 * old per-swap remove+insert path made the algorithm O(n^3).
 */
static int buffer_partition(void **arr, int low, int high, comparator cmp) {
    void *pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (cmp(arr[j], pivot) <= 0) {
            i++;
            buffer_swap(arr, i, j);
        }
    }
    buffer_swap(arr, i + 1, high);
    return i + 1;
}

/**
 * @brief Sorts a list using quicksort over a snapshot pointer buffer.
 *
 * Snapshots the list into a contiguous pointer buffer, sorts it in place
 * with O(1) swaps, then writes the result back via clear+append in O(n).
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list to be sorted.
 */
static void sorter_quick(sorter *self, list *lst) {
    sorter_impl *impl = sorter_impl_from_self(self);
    if (!impl || !impl->cmp) return;

    int size = 0;
    void **arr = list_snapshot(lst, &size);
    if (size < 2) {
        free(arr);
        return;
    }
    if (!arr) return;

    int *stack = malloc(sizeof(int) * (size_t)size * 2);
    if (!stack) {
        free(arr);
        return;
    }

    int top = 0;
    stack[top++] = 0;
    stack[top++] = size - 1;

    while (top > 0) {
        int high = stack[--top];
        int low = stack[--top];
        int pi = buffer_partition(arr, low, high, impl->cmp);

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

    list_replace_with(lst, arr, size);
    free(arr);
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

    list_replace_with(lst, arr, size);

    free(tmp);
    free(arr);
}

/**
 * @brief Shuffle the elements in a list using the Fisher-Yates algorithm.
 *
 * Operates on a snapshot pointer buffer with O(1) swaps and writes the
 * result back via clear+append, giving O(n) total work instead of the
 * O(n^2) cost incurred when swapping through the list interface.
 *
 * @param self A pointer to the sorter structure.
 * @param lst A pointer to the list to be shuffled.
 */
void collections_shuffle(sorter *self, list *lst) {
    (void)self;
    int n = 0;
    void **arr = list_snapshot(lst, &n);
    if (n < 2) {
        free(arr);
        return;
    }
    if (!arr) return;

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        buffer_swap(arr, i, j);
    }

    list_replace_with(lst, arr, n);
    free(arr);
}

/**
 * @brief Reverse the elements in a list.
 *
 * Snapshots the list into a pointer buffer, reverses in place with O(1)
 * swaps, and writes the result back in O(n).
 *
 * @param self A pointer to the sorter instance.
 * @param lst The list to be reversed.
 */
void collections_reverse(sorter *self, list *lst) {
    (void)self;
    int n = 0;
    void **arr = list_snapshot(lst, &n);
    if (n < 2) {
        free(arr);
        return;
    }
    if (!arr) return;

    int i = 0;
    int j = n - 1;
    while (i < j) {
        buffer_swap(arr, i, j);
        i++;
        j--;
    }

    list_replace_with(lst, arr, n);
    free(arr);
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

    int i;
    if (size % 2 == 0) {
        void *a = lst->get(lst, 0);
        void *b = lst->get(lst, 1);
        if (impl && impl->cmp(a, b) > 0) {
            *max = a;
            *min = b;
        } else {
            *min = a;
            *max = b;
        }
        i = 2;
    } else {
        *min = lst->get(lst, 0);
        *max = *min;
        i = 1;
    }

    for (; i + 1 < size; i += 2) {
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
    s->user_data = NULL;

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
