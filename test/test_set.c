#include "base_test.h"
#include "../include/set.h"

#include <stdint.h>

static int int_comparator(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int int_hash(void *key, int capacity) {
    if (!key || capacity <= 0) {
        return 0;
    }
    uint32_t x = (uint32_t)(*(int *)key);
    /* Simple integer mix; ensures equal values hash equally. */
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return (int)(x % (uint32_t)capacity);
}

static int set_contains_int(const set *s, int v) {
    int tmp = v;
    return set_contains(s, &tmp) ? 1 : 0;
}

static int set_remove_int(set *s, int v) {
    int tmp = v;
    return set_remove(s, &tmp) ? 1 : 0;
}

static int list_values_match(const list *lst, const int *expected, int expected_count) {
    if (!lst) {
        return 0;
    }
    if (lst->size(lst) != expected_count) {
        return 0;
    }
    for (int i = 0; i < expected_count; i++) {
        int *p = (int *)lst->get((list *)lst, i);
        if (!p || *p != expected[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_hash_set_basic_ops() {
    set *s = create_set(SET_HASH, int_comparator, int_hash);
    if (!s) {
        print_test_result(0, "Hash set creation");
        return;
    }

    int values[] = {1, 21, 41, 62};
    int ok = 1;
    for (int i = 0; i < 4; i++) {
        ok = ok && set_add(s, &values[i]);
    }
    print_test_result(ok, "Hash set add inserts unique elements");
    print_test_result(set_size(s) == 4, "Hash set size reflects inserted elements");

    int dup = 21;
    print_test_result(!set_add(s, &dup), "Hash set add returns false on duplicate");

    ok = 1;
    for (int i = 0; i < 4; i++) {
        ok = ok && set_contains_int(s, values[i]);
    }
    print_test_result(ok, "Hash set contains finds inserted elements");

    print_test_result(set_remove_int(s, 41), "Hash set remove deletes existing element");
    print_test_result(!set_contains_int(s, 41), "Hash set contains is false after removal");
    print_test_result(set_size(s) == 3, "Hash set size decreases after removal");
    print_test_result(!set_remove_int(s, 41), "Hash set remove returns false when missing");

    list *lst = set_to_list(s);
    print_test_result(lst && lst->size(lst) == 3, "Hash set to_list returns all elements");
    if (lst) {
        lst->free(lst);
    }

    set_free(s);
}

static void test_set_algebra_hash() {
    set *a = create_set(SET_HASH, int_comparator, int_hash);
    set *b = create_set(SET_HASH, int_comparator, int_hash);
    if (!a || !b) {
        print_test_result(0, "Hash set algebra setup");
        set_free(a);
        set_free(b);
        return;
    }

    int av[] = {1, 2, 3};
    int bv[] = {3, 4, 5};
    for (int i = 0; i < 3; i++) {
        set_add(a, &av[i]);
        set_add(b, &bv[i]);
    }

    set *u = set_union(a, b);
    print_test_result(u && set_size(u) == 5 &&
                              set_contains_int(u, 1) && set_contains_int(u, 2) &&
                              set_contains_int(u, 3) && set_contains_int(u, 4) &&
                              set_contains_int(u, 5),
                      "Set union combines unique elements (hash)");

    set *i = set_intersection(a, b);
    print_test_result(i && set_size(i) == 1 && set_contains_int(i, 3), "Set intersection finds common elements (hash)");

    set *d = set_difference(a, b);
    print_test_result(d && set_size(d) == 2 && set_contains_int(d, 1) && set_contains_int(d, 2) && !set_contains_int(d, 3),
                      "Set difference removes elements present in other (hash)");

    print_test_result(i && u && set_is_subset(i, u), "Subset check works (hash)");

    set_free(u);
    set_free(i);
    set_free(d);
    set_free(a);
    set_free(b);
}

static void test_tree_set_basic_ops_and_order() {
    set *s = create_set(SET_TREE, int_comparator, NULL);
    if (!s) {
        print_test_result(0, "Tree set creation");
        return;
    }

    int values[] = {20, 4, 15, 70, 50, 100, 3, 10};
    for (int i = 0; i < 8; i++) {
        set_add(s, &values[i]);
    }
    print_test_result(set_size(s) == 8, "Tree set size reflects inserted elements");
    print_test_result(set_contains_int(s, 70) && set_contains_int(s, 3), "Tree set contains finds inserted elements");

    int expected1[] = {3, 4, 10, 15, 20, 50, 70, 100};
    list *lst = set_to_list(s);
    print_test_result(list_values_match(lst, expected1, 8), "Tree set to_list yields ordered elements");
    if (lst) {
        lst->free(lst);
    }

    print_test_result(set_remove_int(s, 3) && set_remove_int(s, 70) && set_remove_int(s, 20),
                      "Tree set remove deletes existing elements");
    print_test_result(set_size(s) == 5, "Tree set size decreases after removals");

    int expected2[] = {4, 10, 15, 50, 100};
    lst = set_to_list(s);
    print_test_result(list_values_match(lst, expected2, 5), "Tree set remains ordered after removals");
    if (lst) {
        lst->free(lst);
    }

    set_free(s);
}

static void test_set_algebra_tree_ordered_result() {
    set *a = create_set(SET_TREE, int_comparator, NULL);
    set *b = create_set(SET_TREE, int_comparator, NULL);
    if (!a || !b) {
        print_test_result(0, "Tree set algebra setup");
        set_free(a);
        set_free(b);
        return;
    }

    int av[] = {41, 21, 1};
    int bv[] = {21, 62};
    for (int i = 0; i < 3; i++) {
        set_add(a, &av[i]);
    }
    for (int i = 0; i < 2; i++) {
        set_add(b, &bv[i]);
    }

    set *u = set_union(a, b);
    int expected_union[] = {1, 21, 41, 62};
    list *lst = u ? set_to_list(u) : NULL;
    print_test_result(u && list_values_match(lst, expected_union, 4), "Tree set union produces ordered result");
    if (lst) {
        lst->free(lst);
    }

    set_free(u);
    set_free(a);
    set_free(b);
}

static void run_all_tests() {
    test_hash_set_basic_ops();
    test_set_algebra_hash();
    test_tree_set_basic_ops_and_order();
    test_set_algebra_tree_ordered_result();
}

int main() {
    run_all_tests();
    return 0;
}

