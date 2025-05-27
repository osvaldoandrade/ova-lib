#include "base_test.h"
#include "../include/types.h"
#include "../include/sort.h"

#include <stddef.h>  // Para usar o tipo NULL

int int_compare(const void *a, const void *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    return (arg1 > arg2) - (arg1 < arg2);
}

void test_sort_empty_list() {
    list *lst = create_list(ARRAY_LIST, 1);
    sorter *s = create_sorter(lst, int_compare);
    s->sort(s, lst); /* should handle empty list */
    print_test_result(lst->size(lst) == 0, "Sort on empty list safe");
    lst->free(lst);
    free(s);
}


void test_sorter_sort() {
    list *lst = create_list(ARRAY_LIST, 10);
    int items[] = {50, 40, 30, 20, 10};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(lst, int_compare);
    s->sort(s, lst);

    int passed = 1;
    for (int i = 1; i < lst->size(lst); i++) {
        int *prev = (int *)lst->get(lst, i - 1);
        int *current = (int *)lst->get(lst, i);
        if (*prev > *current) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Sorter sort operation ensures list is sorted");

    lst->free(lst);
    free(s);
}

void test_sorter_shuffle() {
    list *lst = create_list(ARRAY_LIST, 10);
    int items[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(lst, int_compare);
    s->shuffle(s, lst);

    int is_same = 1;
    for (int i = 0; i < 10; i++) {
        int *item = (int *)lst->get(lst, i);
        if (*item != items[i]) {
            is_same = 0;
            break;
        }
    }
    print_test_result(!is_same, "Shuffle operation should change the order of items");

    lst->free(lst);
    free(s);
}

void test_sorter_reverse() {
    list *lst = create_list(ARRAY_LIST, 5);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(lst, int_compare);
    s->reverse(s, lst);

    int passed = 1;
    for (int i = 0; i < 5; i++) {
        int *expected = &items[4 - i];
        int *actual = (int *)lst->get(lst, i);
        if (*actual != *expected) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Reverse operation");

    lst->free(lst);
    free(s);
}

void test_sorter_binary_search() {
    list *lst = create_list(ARRAY_LIST, 10);
    int items[] = {50, 40, 30, 20, 10, 60, 70, 80, 90, 100};
    for (int i = 0; i < 10; i++) {
        lst->insert(lst, &items[i], i);
    }
    sorter *s = create_sorter(lst, int_compare);
    s->sort(s, lst);  // Sort the list before binary search

    int search_item = 40;
    int index = s->binary_search(s, lst, &search_item);

    print_test_result(index == 3, "Binary search operation found the correct index");

    lst->free(lst);
    free(s);
}

void test_sorter_copy() {
    list *src = create_list(ARRAY_LIST, 5);
    list *dest = create_list(ARRAY_LIST, 5);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        src->insert(src, &items[i], i);
    }

    sorter *s = create_sorter(src, int_compare);
    s->copy(s, src, dest);

    int passed = 1;
    for (int i = 0; i < src->size(src); i++) {
        int *src_item = (int *)src->get(src, i);
        int *dest_item = (int *)dest->get(dest, i);
        if (*src_item != *dest_item) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Copy operation");

    src->free(src);
    dest->free(dest);
    free(s);
}

void test_sorter_min_max() {
    list *lst = create_list(ARRAY_LIST, 5);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(lst, int_compare);

    // Test min_max
    void *min_val = NULL;
    void *max_val = NULL;
    s->min_max(s, lst, &min_val, &max_val);
    int passed_min_max = (*(int *)min_val == 10) && (*(int *)max_val == 50);
    print_test_result(passed_min_max, "Min Max operation");

    // Test min
    int *min_result = (int *)s->min(s, lst);
    int passed_min = (*min_result == 10);
    print_test_result(passed_min, "Min operation");

    // Test max
    int *max_result = (int *)s->max(s, lst);
    int passed_max = (*max_result == 50);
    print_test_result(passed_max, "Max operation");

    lst->free(lst);
    free(s);
}

void run_all_tests() {
    test_sorter_sort();
    test_sorter_shuffle();
    test_sorter_reverse();
    test_sorter_binary_search();
    test_sorter_copy();
    test_sorter_min_max();
    test_sort_empty_list();
}

int main() {
    run_all_tests();
    return 0;
}
