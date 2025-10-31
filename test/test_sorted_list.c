#include "base_test.h"
#include "../include/list.h"

static int int_comparator(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    if (lhs < rhs) return -1;
    if (lhs > rhs) return 1;
    return 0;
}

static void test_sorted_list_insertion_order() {
    int data[] = {5, 1, 4, 3, 2};
    int expected[] = {1, 2, 3, 4, 5};

    list *lst = create_list(SORTED_LIST, 2, int_comparator);
    if (!lst) {
        print_test_result(0, "Failed to create sorted list");
        return;
    }

    for (int i = 0; i < 5; ++i) {
        lst->insert(lst, &data[i], i);
    }

    int passed = 1;
    for (int i = 0; i < 5; ++i) {
        int *value = (int *)lst->get(lst, i);
        if (!value || *value != expected[i]) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Sorted list maintains order on insert");

    lst->free(lst);
}

static void test_sorted_list_lookup() {
    int data[] = {10, 7, 9, 8};
    int expected[] = {7, 8, 9, 10};

    list *lst = create_list(SORTED_LIST, 4, int_comparator);
    if (!lst) {
        print_test_result(0, "Failed to create sorted list for lookup test");
        return;
    }

    for (int i = 0; i < 4; ++i) {
        lst->insert(lst, &data[i], 0);
    }

    int passed = 1;
    for (int i = 0; i < 4; ++i) {
        int *value = (int *)lst->get(lst, i);
        if (!value || *value != expected[i]) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Sorted list get retrieves ordered values");

    lst->free(lst);
}

static void test_sorted_list_remove() {
    int data[] = {3, 1, 4, 2};
    list *lst = create_list(SORTED_LIST, 2, int_comparator);
    if (!lst) {
        print_test_result(0, "Failed to create sorted list for removal test");
        return;
    }

    for (int i = 0; i < 4; ++i) {
        lst->insert(lst, &data[i], i);
    }

    lst->remove(lst, 1); // remove value 2

    int expected[] = {1, 3, 4};
    int passed = lst->size(lst) == 3;

    for (int i = 0; i < 3 && passed; ++i) {
        int *value = (int *)lst->get(lst, i);
        if (!value || *value != expected[i]) {
            passed = 0;
        }
    }

    print_test_result(passed, "Sorted list remove maintains ordering");

    lst->free(lst);
}

static void run_all_tests() {
    test_sorted_list_insertion_order();
    test_sorted_list_lookup();
    test_sorted_list_remove();
}

int main() {
    run_all_tests();
    return 0;
}
