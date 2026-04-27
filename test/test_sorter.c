#include "base_test.h"
#include "../include/types.h"
#include "../include/sort.h"
#include <time.h>

#include <stddef.h>  // Para usar o tipo NULL

int int_compare(const void *a, const void *b) {
    if (a == NULL || b == NULL) {
        return 0;
    }

    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;

    return (arg1 > arg2) - (arg1 < arg2);
}


void test_sorter_sort(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int items[] = {50, 40, 30, 20, 10};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(int_compare);
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
    s->free(s);
}

void test_sort_empty_list(void) {
    list *lst = create_list(ARRAY_LIST, 1, NULL);
    sorter *s = create_sorter(int_compare);
    s->sort(s, lst);
    print_test_result(lst->size(lst) == 0, "Sort on empty list safe");
    lst->free(lst);
    s->free(s);
}

void test_sorter_shuffle(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int items[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(int_compare);
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
    s->free(s);
}

void test_sorter_reverse(void) {
    list *lst = create_list(ARRAY_LIST, 5, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(int_compare);
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
    s->free(s);
}

void test_sorter_binary_search(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int items[] = {50, 40, 30, 20, 10, 60, 70, 80, 90, 100};
    for (int i = 0; i < 10; i++) {
        lst->insert(lst, &items[i], i);
    }
    sorter *s = create_sorter(int_compare);
    s->sort(s, lst);  // Sort the list before binary search

    int search_item = 40;
    int index = s->binary_search(s, lst, &search_item);

    print_test_result(index == 3, "Binary search operation found the correct index");

    lst->free(lst);
    s->free(s);
}

void test_sorter_large_sort(void) {
    const int MAX = 1000;
    list *lst = create_list(ARRAY_LIST, MAX, NULL);
    int *values = malloc((size_t)MAX * sizeof(int));
    if (values == NULL) {
        print_test_result(0, "Large sort within time limit");
        lst->free(lst);
        return;
    }
    for(int i = 0; i < MAX; i++){
        values[i] = MAX - 1 - i;
        lst->insert(lst, &values[i], i);
    }
    sorter *s = create_sorter(int_compare);
    clock_t start = clock();
    s->sort(s, lst);
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    int sorted = 1;
    for(int i=1;i<lst->size(lst);i++){
        int *prev = lst->get(lst,i-1);
        int *cur = lst->get(lst,i);
        if(*prev > *cur){sorted=0;break;}
    }
    print_test_result(sorted && elapsed_ms < 1500.0, "Large sort within time limit");
    lst->free(lst);
    free(values);
    s->free(s);
}

void test_sorter_copy(void) {
    list *src = create_list(ARRAY_LIST, 5, NULL);
    list *dest = create_list(ARRAY_LIST, 5, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        src->insert(src, &items[i], i);
    }

    sorter *s = create_sorter(int_compare);
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
    s->free(s);
}

void test_sorter_min_max(void) {
    list *lst = create_list(ARRAY_LIST, 5, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_sorter(int_compare);

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
    s->free(s);
}

void test_merge_sorter_sort(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int items[] = {50, 40, 30, 20, 10};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_merge_sorter(int_compare);
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
    print_test_result(passed, "Merge sorter sort operation ensures list is sorted");

    lst->free(lst);
    s->free(s);
}

void test_merge_sort_empty_list(void) {
    list *lst = create_list(ARRAY_LIST, 1, NULL);
    sorter *s = create_merge_sorter(int_compare);
    s->sort(s, lst);
    print_test_result(lst->size(lst) == 0, "Merge sort on empty list safe");
    lst->free(lst);
    s->free(s);
}

void test_merge_sort_single_element(void) {
    list *lst = create_list(ARRAY_LIST, 1, NULL);
    int item = 42;
    lst->insert(lst, &item, 0);

    sorter *s = create_merge_sorter(int_compare);
    s->sort(s, lst);

    int *result = (int *)lst->get(lst, 0);
    print_test_result(*result == 42, "Merge sort on single element");
    lst->free(lst);
    s->free(s);
}

void test_merge_sort_already_sorted(void) {
    list *lst = create_list(ARRAY_LIST, 5, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }

    sorter *s = create_merge_sorter(int_compare);
    s->sort(s, lst);

    int passed = 1;
    for (int i = 0; i < 5; i++) {
        int *val = (int *)lst->get(lst, i);
        if (*val != items[i]) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Merge sort on already sorted list");
    lst->free(lst);
    s->free(s);
}

void test_merge_sort_large(void) {
    const int MAX = 1000;
    list *lst = create_list(ARRAY_LIST, MAX, NULL);
    int *values = malloc((size_t)MAX * sizeof(int));
    if (values == NULL) {
        print_test_result(0, "Merge sort large within time limit");
        lst->free(lst);
        return;
    }
    for (int i = 0; i < MAX; i++) {
        values[i] = MAX - 1 - i;
        lst->insert(lst, &values[i], i);
    }
    sorter *s = create_merge_sorter(int_compare);
    clock_t start = clock();
    s->sort(s, lst);
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    int sorted = 1;
    for (int i = 1; i < lst->size(lst); i++) {
        int *prev = lst->get(lst, i - 1);
        int *cur = lst->get(lst, i);
        if (*prev > *cur) { sorted = 0; break; }
    }
    print_test_result(sorted && elapsed_ms < 1500.0, "Merge sort large within time limit");
    lst->free(lst);
    free(values);
    s->free(s);
}

void run_all_tests(void) {
    test_sorter_sort();
    test_sorter_shuffle();
    test_sorter_reverse();
    test_sorter_binary_search();
    test_sort_empty_list();
    test_sorter_large_sort();
    test_sorter_copy();
    test_sorter_min_max();
    test_merge_sorter_sort();
    test_merge_sort_empty_list();
    test_merge_sort_single_element();
    test_merge_sort_already_sorted();
    test_merge_sort_large();
}

int main(void) {
    run_all_tests();
    return 0;
}
