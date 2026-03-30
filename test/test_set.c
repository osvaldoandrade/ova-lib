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
    return s->contains(s, &tmp) ? 1 : 0;
}

static int set_remove_int(set *s, int v) {
    int tmp = v;
    return s->remove(s, &tmp) ? 1 : 0;
}

static int list_values_match(list *lst, const int *expected, int expected_count) {
    if (!lst) {
        return 0;
    }
    if (lst->size(lst) != expected_count) {
        return 0;
    }
    for (int i = 0; i < expected_count; i++) {
        int *p = (int *)lst->get(lst, i);
        if (!p || *p != expected[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_hash_set_basic_ops(void) {
    set *s = create_set(SET_HASH, int_comparator, int_hash);
    if (!s) {
        print_test_result(0, "Hash set creation");
        return;
    }

    int values[] = {1, 21, 41, 62};
    int ok = 1;
    for (int i = 0; i < 4; i++) {
        ok = ok && s->add(s, &values[i]);
    }
    print_test_result(ok, "Hash set add inserts unique elements");
    print_test_result(s->size(s) == 4, "Hash set size reflects inserted elements");

    int dup = 21;
    print_test_result(!s->add(s, &dup), "Hash set add returns false on duplicate");

    ok = 1;
    for (int i = 0; i < 4; i++) {
        ok = ok && set_contains_int(s, values[i]);
    }
    print_test_result(ok, "Hash set contains finds inserted elements");

    print_test_result(set_remove_int(s, 41), "Hash set remove deletes existing element");
    print_test_result(!set_contains_int(s, 41), "Hash set contains is false after removal");
    print_test_result(s->size(s) == 3, "Hash set size decreases after removal");
    print_test_result(!set_remove_int(s, 41), "Hash set remove returns false when missing");

    list *lst = s->to_list(s);
    print_test_result(lst && lst->size(lst) == 3, "Hash set to_list returns all elements");
    if (lst) {
        lst->free(lst);
    }

    if (s) { s->free(s); }
}

static void test_set_algebra_hash(void) {
    set *a = create_set(SET_HASH, int_comparator, int_hash);
    set *b = create_set(SET_HASH, int_comparator, int_hash);
    if (!a || !b) {
        print_test_result(0, "Hash set algebra setup");
        if (a) { a->free(a); }
        if (b) { b->free(b); }
        return;
    }

    int av[] = {1, 2, 3};
    int bv[] = {3, 4, 5};
    for (int i = 0; i < 3; i++) {
        a->add(a, &av[i]);
        b->add(b, &bv[i]);
    }

    set *u = a->union_with(a, b);
    print_test_result(u && u->size(u) == 5 &&
                              set_contains_int(u, 1) && set_contains_int(u, 2) &&
                              set_contains_int(u, 3) && set_contains_int(u, 4) &&
                              set_contains_int(u, 5),
                      "Set union combines unique elements (hash)");

    set *i = a->intersection_with(a, b);
    print_test_result(i && i->size(i) == 1 && set_contains_int(i, 3), "Set intersection finds common elements (hash)");

    set *d = a->difference_with(a, b);
    print_test_result(d && d->size(d) == 2 && set_contains_int(d, 1) && set_contains_int(d, 2) && !set_contains_int(d, 3),
                      "Set difference removes elements present in other (hash)");

    print_test_result(i && u && i->is_subset_of(i, u), "Subset check works (hash)");

    if (u) { u->free(u); }
    if (i) { i->free(i); }
    if (d) { d->free(d); }
    if (a) { a->free(a); }
    if (b) { b->free(b); }
}

static void test_tree_set_basic_ops_and_order(void) {
    set *s = create_set(SET_TREE, int_comparator, NULL);
    if (!s) {
        print_test_result(0, "Tree set creation");
        return;
    }

    int values[] = {20, 4, 15, 70, 50, 100, 3, 10};
    for (int i = 0; i < 8; i++) {
        s->add(s, &values[i]);
    }
    print_test_result(s->size(s) == 8, "Tree set size reflects inserted elements");
    print_test_result(set_contains_int(s, 70) && set_contains_int(s, 3), "Tree set contains finds inserted elements");

    int expected1[] = {3, 4, 10, 15, 20, 50, 70, 100};
    list *lst = s->to_list(s);
    print_test_result(list_values_match(lst, expected1, 8), "Tree set to_list yields ordered elements");
    if (lst) {
        lst->free(lst);
    }

    print_test_result(set_remove_int(s, 3) && set_remove_int(s, 70) && set_remove_int(s, 20),
                      "Tree set remove deletes existing elements");
    print_test_result(s->size(s) == 5, "Tree set size decreases after removals");

    int expected2[] = {4, 10, 15, 50, 100};
    lst = s->to_list(s);
    print_test_result(list_values_match(lst, expected2, 5), "Tree set remains ordered after removals");
    if (lst) {
        lst->free(lst);
    }

    if (s) { s->free(s); }
}

static void test_set_algebra_tree_ordered_result(void) {
    set *a = create_set(SET_TREE, int_comparator, NULL);
    set *b = create_set(SET_TREE, int_comparator, NULL);
    if (!a || !b) {
        print_test_result(0, "Tree set algebra setup");
        if (a) { a->free(a); }
        if (b) { b->free(b); }
        return;
    }

    int av[] = {41, 21, 1};
    int bv[] = {21, 62};
    for (int i = 0; i < 3; i++) {
        a->add(a, &av[i]);
    }
    for (int i = 0; i < 2; i++) {
        b->add(b, &bv[i]);
    }

    set *u = a->union_with(a, b);
    int expected_union[] = {1, 21, 41, 62};
    list *lst = u ? u->to_list(u) : NULL;
    print_test_result(u && list_values_match(lst, expected_union, 4), "Tree set union produces ordered result");
    if (lst) {
        lst->free(lst);
    }

    if (u) { u->free(u); }
    if (a) { a->free(a); }
    if (b) { b->free(b); }
}

static void test_hash_set_add_bulk(void) {
    set *s = create_set(SET_HASH, int_comparator, int_hash);
    if (!s) {
        print_test_result(0, "Hash set creation for bulk test");
        return;
    }

    int values[] = {10, 20, 30, 40, 50};
    void *ptrs[] = {&values[0], &values[1], &values[2], &values[3], &values[4]};

    s->add_bulk(s, ptrs, 5);
    print_test_result(s->size(s) == 5, "Hash set bulk add inserts all elements");

    int ok = 1;
    for (int i = 0; i < 5; i++) {
        ok = ok && s->contains(s, &values[i]);
    }
    print_test_result(ok, "Hash set bulk add elements are all findable");

    if (s) { s->free(s); }
}

static void test_tree_set_add_bulk(void) {
    set *s = create_set(SET_TREE, int_comparator, NULL);
    if (!s) {
        print_test_result(0, "Tree set creation for bulk test");
        return;
    }

    int values[] = {50, 10, 40, 20, 30};
    void *ptrs[] = {&values[0], &values[1], &values[2], &values[3], &values[4]};

    s->add_bulk(s, ptrs, 5);
    print_test_result(s->size(s) == 5, "Tree set bulk add inserts all elements");

    int expected[] = {10, 20, 30, 40, 50};
    list *lst = s->to_list(s);
    print_test_result(list_values_match(lst, expected, 5), "Tree set bulk add maintains order");
    if (lst) {
        lst->free(lst);
    }

    if (s) { s->free(s); }
}

static void test_set_add_bulk_with_duplicates(void) {
    set *s = create_set(SET_HASH, int_comparator, int_hash);
    if (!s) {
        print_test_result(0, "Hash set creation for bulk duplicate test");
        return;
    }

    int values[] = {10, 20, 10, 30, 20};
    void *ptrs[] = {&values[0], &values[1], &values[2], &values[3], &values[4]};

    s->add_bulk(s, ptrs, 5);
    print_test_result(s->size(s) == 3, "Set bulk add skips duplicates");

    if (s) { s->free(s); }
}

static void test_set_add_bulk_edge_cases(void) {
    set *s = create_set(SET_HASH, int_comparator, int_hash);
    int v = 1;
    void *ptrs[] = {&v};

    s->add_bulk(s, ptrs, 0);
    print_test_result(s->size(s) == 0, "Set bulk add with count 0 does nothing");

    s->add_bulk(s, NULL, 3);
    print_test_result(s->size(s) == 0, "Set bulk add with NULL elements does nothing");

    s->add_bulk(NULL, ptrs, 1);
    print_test_result(1, "Set bulk add with NULL set does not crash");

    if (s) { s->free(s); }
}

static void run_all_tests(void) {
    test_hash_set_basic_ops();
    test_set_algebra_hash();
    test_tree_set_basic_ops_and_order();
    test_set_algebra_tree_ordered_result();
    test_hash_set_add_bulk();
    test_tree_set_add_bulk();
    test_set_add_bulk_with_duplicates();
    test_set_add_bulk_edge_cases();
}

int main(void) {
    run_all_tests();
    return 0;
}
