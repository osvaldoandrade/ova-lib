#include "base_test.h"
#include "../include/heap.h"
#include <time.h>

int int_compare(const void *a, const void *b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

void test_heap_insert_and_extract_max() {
    heap *h = create_heap(BINARY_HEAP,10, int_compare);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->pop(h);
    print_test_result(*max == 10, "Heap put and extract max");
    h->free(h);
}

void test_heap_peek_max() {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->pop(h);
    print_test_result(*max == 10, "Heap peek max");
    h->free(h);
}

void test_heap_empty_check() {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    print_test_result(h->size(h) == 0, "Heap empty on creation"); // Usar função size() aqui
    int value = 10;
    h->put(h, &value);
    print_test_result(h->size(h) != 0, "Heap not empty after put"); // Usar função size() aqui
    h->free(h);
}

void test_heap_pop_empty() {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    print_test_result(h->pop(h) == NULL, "Pop on empty heap returns NULL");
    h->free(h);
}

void test_heap_high_volume() {
    heap *h = create_heap(BINARY_HEAP, 10000, int_compare);
    const int MAX = 1000;
    clock_t start = clock();
    for (int i = 0; i < MAX; i++) {
        h->put(h, &i);
    }
    for (int i = 0; i < MAX; i++) {
        h->pop(h);
    }
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1500.0 && h->size(h) == 0, "Heap high volume within time limit");
    h->free(h);
}

void run_all_heap_tests() {
    test_heap_insert_and_extract_max();
    test_heap_peek_max();
    test_heap_empty_check();
    test_heap_pop_empty();
    test_heap_high_volume();
}

int main() {
    run_all_heap_tests();
    return 0;
}
