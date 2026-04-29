#include "base_test.h"
#include "../include/heap.h"
#include "../src/utils/capacity_utils.h"
#include <limits.h>
#include <time.h>

int int_compare(const void *a, const void *b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

void test_heap_insert_and_extract_max(void) {
    heap *h = create_heap(BINARY_HEAP,10, int_compare);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < (int)(sizeof(values) / sizeof(values[0])); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->pop(h);
    print_test_result(*max == 10, "Heap put and extract max");
    h->free(h);
}

void test_heap_peek_max(void) {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < (int)(sizeof(values) / sizeof(values[0])); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->peek(h);
    print_test_result(*max == 10, "Heap peek max");
    h->free(h);
}

void test_heap_empty_check(void) {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    print_test_result(h->size(h) == 0, "Heap empty on creation"); // Usar função size() aqui
    int value = 10;
    h->put(h, &value);
    print_test_result(h->size(h) != 0, "Heap not empty after put"); // Usar função size() aqui
    h->free(h);
}

void test_heap_pop_empty(void) {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    print_test_result(h->pop(h) == NULL, "Pop on empty heap returns NULL");
    h->free(h);
}

void test_heap_high_volume(void) {
    heap *h = create_heap(BINARY_HEAP, 10000, int_compare);
    enum { MAX = 1000 };
    int values[MAX];
    clock_t start = clock();
    for (int i = 0; i < MAX; i++) {
        values[i] = i;
        h->put(h, &values[i]);
    }
    for (int i = 0; i < MAX; i++) {
        h->pop(h);
    }
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1500.0 && h->size(h) == 0, "Heap high volume within time limit");
    h->free(h);
}

void test_safe_double_capacity_for_binary_heap(void) {
    print_test_result(safe_double_capacity(10) == 20, "Heap safe_double_capacity(10) == 20");
    print_test_result(safe_double_capacity(INT_MAX / 2 + 1) == INT_MAX, "Heap safe_double_capacity caps at INT_MAX");
    print_test_result(safe_double_capacity(INT_MAX) == INT_MAX, "Heap safe_double_capacity(INT_MAX) stays INT_MAX");
}

void test_heap_put_error_codes(void) {
    heap *h = create_heap(BINARY_HEAP, 4, int_compare);
    int val = 42;

    print_test_result(h->put(h, &val) == OVA_SUCCESS,
                      "Binary heap put returns OVA_SUCCESS");

    h->free(h);
}

void run_all_heap_tests(void) {
    test_safe_double_capacity_for_binary_heap();
    test_heap_insert_and_extract_max();
    test_heap_peek_max();
    test_heap_empty_check();
    test_heap_pop_empty();
    test_heap_high_volume();
    test_heap_put_error_codes();
}

int main(void) {
    run_all_heap_tests();
    return 0;
}
