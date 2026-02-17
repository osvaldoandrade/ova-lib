#include "base_test.h"
#include "../include/bloom_filter.h"

#include <string.h>

static void test_bloom_filter_create_validation() {
    bloom_filter *bf = create_bloom_filter(0, 0.01);
    print_test_result(bf == NULL, "Bloom filter rejects expected_elements <= 0");
    bloom_filter_free(bf);

    bf = create_bloom_filter(10, 0.0);
    print_test_result(bf == NULL, "Bloom filter rejects false_positive_rate <= 0");
    bloom_filter_free(bf);

    bf = create_bloom_filter(10, 1.0);
    print_test_result(bf == NULL, "Bloom filter rejects false_positive_rate >= 1");
    bloom_filter_free(bf);
}

static void test_bloom_filter_basic_ops() {
    bloom_filter *bf = create_bloom_filter(100, 0.01);
    if (!bf) {
        print_test_result(0, "Bloom filter creation");
        return;
    }

    const char *k1 = "hello";
    const char *k2 = "world";

    bloom_filter_add(bf, k1, strlen(k1));
    bloom_filter_add(bf, k2, strlen(k2));

    print_test_result(bloom_filter_might_contain(bf, k1, strlen(k1)), "Bloom filter has no false negatives (k1)");
    print_test_result(bloom_filter_might_contain(bf, k2, strlen(k2)), "Bloom filter has no false negatives (k2)");

    unsigned char bytes[] = {0x00, 0x01, 0x02, 0x03, 0xFF};
    bloom_filter_add(bf, bytes, sizeof(bytes));
    print_test_result(bloom_filter_might_contain(bf, bytes, sizeof(bytes)), "Bloom filter supports binary keys");

    double fpp_before = bloom_filter_current_fpp(bf);
    int fpp_ok = (fpp_before >= 0.0) && (fpp_before <= 1.0);
    print_test_result(fpp_ok, "Bloom filter current_fpp returns value in [0,1]");
    print_test_result(fpp_before > 0.0, "Bloom filter current_fpp increases after inserts");

    bloom_filter_clear(bf);

    print_test_result(!bloom_filter_might_contain(bf, k1, strlen(k1)), "Bloom filter clear resets membership (k1)");
    print_test_result(!bloom_filter_might_contain(bf, k2, strlen(k2)), "Bloom filter clear resets membership (k2)");

    double fpp_after = bloom_filter_current_fpp(bf);
    print_test_result(fpp_after == 0.0, "Bloom filter current_fpp is 0 after clear");

    bloom_filter_free(bf);
}

static void run_all_tests() {
    test_bloom_filter_create_validation();
    test_bloom_filter_basic_ops();
}

int main() {
    run_all_tests();
    return 0;
}

