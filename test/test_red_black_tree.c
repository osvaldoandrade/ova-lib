#include "base_test.h"
#include "../include/tree.h"

typedef struct {
    int key;
    int value;
} kv_pair;

static int int_comparator(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int collected_keys[64];
static int collected_count = 0;

static void collect_key_cb(void *key, void *value) {
    (void)value;
    if (collected_count < (int)(sizeof(collected_keys) / sizeof(collected_keys[0]))) {
        collected_keys[collected_count++] = *(int *)key;
    }
}

static int assert_in_order_keys(tree *t, const int *expected, int expected_count) {
    collected_count = 0;
    tree_in_order_traverse(t, collect_key_cb);
    if (collected_count != expected_count) {
        return 0;
    }
    for (int i = 0; i < expected_count; i++) {
        if (collected_keys[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}

static void test_red_black_tree_basic_ops() {
    tree *t = create_tree(TREE_RED_BLACK, int_comparator);
    if (!t) {
        print_test_result(0, "Red-Black tree creation");
        return;
    }

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
        tree_insert(t, &items[i].key, &items[i].value);
    }

    int query = 50;
    int *found = (int *)tree_search(t, &query);
    print_test_result(found && *found == 500, "RB tree search returns correct value");

    int update_key = 15;
    int update_value = 999;
    tree_insert(t, &update_key, &update_value);
    int *updated = (int *)tree_search(t, &update_key);
    print_test_result(updated && *updated == 999, "RB tree insert updates existing key value");

    int *minv = (int *)tree_min(t);
    int *maxv = (int *)tree_max(t);
    print_test_result(minv && *minv == 30, "RB tree min returns smallest key value");
    print_test_result(maxv && *maxv == 1000, "RB tree max returns largest key value");

    int pred_key = 15;
    int succ_key = 15;
    int *pred = (int *)tree_predecessor(t, &pred_key);
    int *succ = (int *)tree_successor(t, &succ_key);
    print_test_result(pred && *pred == 100, "RB predecessor returns correct value");
    print_test_result(succ && *succ == 200, "RB successor returns correct value");

    int missing_key = 16;
    int *pred2 = (int *)tree_predecessor(t, &missing_key);
    int *succ2 = (int *)tree_successor(t, &missing_key);
    print_test_result(pred2 && *pred2 == 999, "RB predecessor works for missing key");
    print_test_result(succ2 && *succ2 == 200, "RB successor works for missing key");

    int low = 10;
    int high = 50;
    list *range = tree_range_query(t, &low, &high);
    int range_ok = range && range->size(range) == 4;
    if (range_ok) {
        int expected_values[] = {100, 999, 200, 500};
        for (int i = 0; i < 4; i++) {
            int *v = (int *)range->get(range, i);
            if (!v || *v != expected_values[i]) {
                range_ok = 0;
                break;
            }
        }
    }
    print_test_result(range_ok, "RB range query returns ordered values");
    if (range) {
        range->free(range);
    }

    int expected_keys1[] = {3, 4, 10, 15, 20, 50, 70, 100};
    print_test_result(assert_in_order_keys(t, expected_keys1, 8), "RB in-order traversal yields sorted keys");

    int delete_key = 3;
    tree_delete(t, &delete_key);
    print_test_result(tree_search(t, &delete_key) == NULL, "RB delete removes leaf node");

    int delete_key2 = 70;
    tree_delete(t, &delete_key2);
    print_test_result(tree_search(t, &delete_key2) == NULL, "RB delete removes internal node");

    int delete_key3 = 20;
    tree_delete(t, &delete_key3);
    print_test_result(tree_search(t, &delete_key3) == NULL, "RB delete removes two-child node");

    int expected_keys2[] = {4, 10, 15, 50, 100};
    print_test_result(assert_in_order_keys(t, expected_keys2, 5), "RB traversal remains sorted after deletes");
    print_test_result(tree_size(t) == 5, "RB tree_size matches remaining items");

    int missing_delete = 123;
    tree_delete(t, &missing_delete);
    print_test_result(tree_size(t) == 5, "RB delete on missing key is a no-op");

    tree_free(t);
}

int main() {
    test_red_black_tree_basic_ops();
    return 0;
}

