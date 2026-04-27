#include "base_test.h"
#include "../include/skip_list.h"

typedef struct {
    int key;
    int value;
} kv_pair;

static int int_comparator(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

/* ------------------------------------------------------------------ */
/*  Creation tests                                                     */
/* ------------------------------------------------------------------ */

static void test_create_skip_list(void) {
    skip_list *sl = create_skip_list(16, int_comparator);
    assert_not_null(sl);
    assert_int_equal(0, sl->size(sl));
    sl->free(sl);
    print_test_result(1, "skip list creation");
}

static void test_create_skip_list_null_cmp(void) {
    skip_list *sl = create_skip_list(16, NULL);
    print_test_result(sl == NULL, "skip list creation with NULL comparator returns NULL");
}

static void test_create_skip_list_bad_level(void) {
    skip_list *sl = create_skip_list(0, int_comparator);
    print_test_result(sl == NULL, "skip list creation with max_level=0 returns NULL");
}

/* ------------------------------------------------------------------ */
/*  Insert / search tests                                              */
/* ------------------------------------------------------------------ */

static void test_insert_and_search(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    kv_pair items[] = {
        {20, 200},
        {4, 40},
        {15, 150},
        {70, 700},
        {50, 500},
        {100, 1000},
        {3, 30},
        {10, 100},
    };
    const int n = (int)(sizeof(items) / sizeof(items[0]));

    for (int i = 0; i < n; i++) {
        sl->insert(sl, &items[i].key, &items[i].value);
    }

    assert_int_equal(n, sl->size(sl));
    print_test_result(sl->size(sl) == n, "skip list size after inserts");

    int query = 50;
    int *found = (int *)sl->search(sl, &query);
    print_test_result(found && *found == 500, "skip list search returns correct value");

    int missing = 999;
    print_test_result(sl->search(sl, &missing) == NULL, "skip list search for missing key returns NULL");

    sl->free(sl);
}

static void test_insert_update(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    kv_pair a = {10, 100};
    sl->insert(sl, &a.key, &a.value);

    int update_value = 999;
    sl->insert(sl, &a.key, &update_value);

    int *found = (int *)sl->search(sl, &a.key);
    print_test_result(found && *found == 999, "skip list insert updates existing key value");
    assert_int_equal(1, sl->size(sl));
    print_test_result(sl->size(sl) == 1, "skip list size unchanged after update");

    sl->free(sl);
}

/* ------------------------------------------------------------------ */
/*  Delete tests                                                       */
/* ------------------------------------------------------------------ */

static void test_delete(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    kv_pair items[] = {
        {20, 200},
        {4, 40},
        {15, 150},
        {70, 700},
        {50, 500},
    };
    const int n = (int)(sizeof(items) / sizeof(items[0]));

    for (int i = 0; i < n; i++) {
        sl->insert(sl, &items[i].key, &items[i].value);
    }

    ova_error_code rc = sl->delete(sl, &items[2].key); /* delete 15 */
    print_test_result(rc == OVA_SUCCESS, "skip list delete returns OVA_SUCCESS");
    print_test_result(sl->search(sl, &items[2].key) == NULL, "skip list deleted key not found");
    assert_int_equal(n - 1, sl->size(sl));
    print_test_result(sl->size(sl) == n - 1, "skip list size decremented after delete");

    /* Remaining keys still accessible */
    int *v20 = (int *)sl->search(sl, &items[0].key);
    int *v4  = (int *)sl->search(sl, &items[1].key);
    int *v70 = (int *)sl->search(sl, &items[3].key);
    int *v50 = (int *)sl->search(sl, &items[4].key);
    print_test_result(v20 && *v20 == 200, "skip list remaining key 20 intact");
    print_test_result(v4  && *v4  == 40,  "skip list remaining key 4 intact");
    print_test_result(v70 && *v70 == 700, "skip list remaining key 70 intact");
    print_test_result(v50 && *v50 == 500, "skip list remaining key 50 intact");

    sl->free(sl);
}

static void test_delete_missing_key(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    kv_pair a = {10, 100};
    sl->insert(sl, &a.key, &a.value);

    int missing = 42;
    ova_error_code rc = sl->delete(sl, &missing);
    print_test_result(rc == OVA_ERROR_NOT_FOUND, "skip list delete missing key returns OVA_ERROR_NOT_FOUND");
    assert_int_equal(1, sl->size(sl));

    sl->free(sl);
}

/* ------------------------------------------------------------------ */
/*  Error-code tests                                                   */
/* ------------------------------------------------------------------ */

static void test_error_codes(void) {
    skip_list *sl = create_skip_list(16, int_comparator);
    int k = 42, v = 99;

    print_test_result(sl->insert(sl, &k, &v) == OVA_SUCCESS,
                      "skip list insert returns OVA_SUCCESS");
    print_test_result(sl->delete(sl, &k) == OVA_SUCCESS,
                      "skip list delete returns OVA_SUCCESS");

    sl->free(sl);
}

/* ------------------------------------------------------------------ */
/*  Stress / many-elements test                                        */
/* ------------------------------------------------------------------ */

static void test_many_elements(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    enum { COUNT = 200 };
    int keys[COUNT];
    int values[COUNT];

    for (int i = 0; i < COUNT; i++) {
        keys[i]   = i;
        values[i] = i * 10;
        sl->insert(sl, &keys[i], &values[i]);
    }

    assert_int_equal(COUNT, sl->size(sl));

    /* Verify all elements */
    int all_found = 1;
    for (int i = 0; i < COUNT; i++) {
        int *found = (int *)sl->search(sl, &keys[i]);
        if (!found || *found != i * 10) {
            all_found = 0;
            break;
        }
    }
    print_test_result(all_found, "skip list search all 200 elements");

    /* Delete every other element */
    for (int i = 0; i < COUNT; i += 2) {
        sl->delete(sl, &keys[i]);
    }
    assert_int_equal(COUNT / 2, sl->size(sl));

    /* Deleted elements are gone, kept elements remain */
    int delete_ok = 1;
    for (int i = 0; i < COUNT; i++) {
        int *found = (int *)sl->search(sl, &keys[i]);
        if (i % 2 == 0) {
            if (found != NULL) { delete_ok = 0; break; }
        } else {
            if (!found || *found != i * 10) { delete_ok = 0; break; }
        }
    }
    print_test_result(delete_ok, "skip list delete alternating elements correct");

    sl->free(sl);
}

/* ------------------------------------------------------------------ */
/*  Empty list edge cases                                              */
/* ------------------------------------------------------------------ */

static void test_empty_list(void) {
    skip_list *sl = create_skip_list(16, int_comparator);

    assert_int_equal(0, sl->size(sl));

    int key = 1;
    print_test_result(sl->search(sl, &key) == NULL, "skip list search on empty returns NULL");

    ova_error_code rc = sl->delete(sl, &key);
    print_test_result(rc == OVA_ERROR_NOT_FOUND, "skip list delete on empty returns OVA_ERROR_NOT_FOUND");

    sl->free(sl);
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */

int main(void) {
    test_create_skip_list();
    test_create_skip_list_null_cmp();
    test_create_skip_list_bad_level();
    test_insert_and_search();
    test_insert_update();
    test_delete();
    test_delete_missing_key();
    test_error_codes();
    test_many_elements();
    test_empty_list();
    return 0;
}
