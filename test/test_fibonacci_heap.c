#include "base_test.h"
#include "../include/heap.h"
#include <time.h>

int int_compare_fibonacci(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ib - ia); // Para max-heap; inverter para min-heap
}

void test_heap_insert_and_extract_max() {
    heap *h = create_heap(FIBONACCI_HEAP,10, int_compare_fibonacci);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->pop(h);
    print_test_result(*max == 10, "Heap put and extract max");
    h->free(h);
}

void test_heap_peek_max() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        h->put(h, &values[i]);
    }
    int *max = (int *)h->pop(h);
    print_test_result(*max == 10, "Heap peek max");
    h->free(h);
}

void test_heap_empty_check() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    print_test_result(h->size(h) == 0, "Heap empty on creation"); // Usar função size() aqui
    int value = 10;
    h->put(h, &value);
    print_test_result(h->size(h) != 0, "Heap not empty after put"); // Usar função size() aqui
    h->free(h);
}

void test_heap_pop_empty() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    print_test_result(h->pop(h) == NULL, "Pop on empty fibonacci heap returns NULL");
    h->free(h);
}

void test_heap_high_volume() {
    heap *h = create_heap(FIBONACCI_HEAP, 10000, int_compare_fibonacci);
    const int MAX = 1000;
    clock_t start = clock();
    for (int i = 0; i < MAX; i++) {
        h->put(h, &i);
    }
    for (int i = 0; i < MAX; i++) {
        h->pop(h);
    }
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1500.0 && h->size(h) == 0, "Fibonacci heap high volume within time limit");
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
