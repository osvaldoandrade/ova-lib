#include "base_test.h"
#include "../include/bloom_filter.h"

#include <string.h>

static void test_bloom_filter_create_validation(void) {
    bloom_filter *bf = create_bloom_filter(0, 0.01);
    print_test_result(bf == NULL, "Bloom filter rejects expected_elements <= 0");
    if (bf) {
        bf->free(bf);
    }

    bf = create_bloom_filter(10, 0.0);
    print_test_result(bf == NULL, "Bloom filter rejects false_positive_rate <= 0");
    if (bf) {
        bf->free(bf);
    }

    bf = create_bloom_filter(10, 1.0);
    print_test_result(bf == NULL, "Bloom filter rejects false_positive_rate >= 1");
    if (bf) {
        bf->free(bf);
    }
}

static void test_bloom_filter_basic_ops(void) {
    bloom_filter *bf = create_bloom_filter(100, 0.01);
    if (!bf) {
        print_test_result(0, "Bloom filter creation");
        return;
    }

    const char *k1 = "hello";
    const char *k2 = "world";

    bf->add(bf, k1, strlen(k1));
    bf->add(bf, k2, strlen(k2));

    print_test_result(bf->might_contain(bf, k1, strlen(k1)), "Bloom filter has no false negatives (k1)");
    print_test_result(bf->might_contain(bf, k2, strlen(k2)), "Bloom filter has no false negatives (k2)");

    unsigned char bytes[] = {0x00, 0x01, 0x02, 0x03, 0xFF};
    bf->add(bf, bytes, sizeof(bytes));
    print_test_result(bf->might_contain(bf, bytes, sizeof(bytes)), "Bloom filter supports binary keys");

    double fpp_before = bf->current_fpp(bf);
    int fpp_ok = (fpp_before >= 0.0) && (fpp_before <= 1.0);
    print_test_result(fpp_ok, "Bloom filter current_fpp returns value in [0,1]");
    print_test_result(fpp_before > 0.0, "Bloom filter current_fpp increases after inserts");

    bf->clear(bf);

    print_test_result(!bf->might_contain(bf, k1, strlen(k1)), "Bloom filter clear resets membership (k1)");
    print_test_result(!bf->might_contain(bf, k2, strlen(k2)), "Bloom filter clear resets membership (k2)");

    double fpp_after = bf->current_fpp(bf);
    print_test_result(fpp_after == 0.0, "Bloom filter current_fpp is 0 after clear");

    bf->free(bf);
}

static void test_bloom_filter_empty_keys(void) {
    bloom_filter *bf = create_bloom_filter(100, 0.01);
    if (!bf) {
        print_test_result(0, "Bloom filter creation");
        return;
    }

    /* Test that empty keys (len == 0) are supported - no false negatives */
    bf->add(bf, NULL, 0);
    bool result1 = bf->might_contain(bf, NULL, 0);
    print_test_result(result1, "Bloom filter supports empty keys (NULL, 0) - no false negative");

    /* Empty key with non-NULL pointer should also work - no false negatives */
    const char *dummy = "dummy";
    bf->add(bf, dummy, 0);
    bool result2 = bf->might_contain(bf, dummy, 0);
    print_test_result(result2, "Bloom filter supports empty keys (ptr, 0) - no false negative");

    /* Ensure empty keys are considered the same regardless of pointer - no false negatives */
    bool result3 = bf->might_contain(bf, NULL, 0);
    print_test_result(result3, "Bloom filter treats all empty keys consistently - no false negative");

    /* Verify non-empty keys still work correctly after adding empty keys */
    const char *key1 = "test";
    bf->add(bf, key1, strlen(key1));
    bool has_key1 = bf->might_contain(bf, key1, strlen(key1));
    print_test_result(has_key1, "Bloom filter works for non-empty keys after empty key insert");

    bf->free(bf);
}

static void run_all_tests(void) {
    test_bloom_filter_create_validation();
    test_bloom_filter_basic_ops();
    test_bloom_filter_empty_keys();
}

int main(void) {
    run_all_tests();
    return 0;
}
