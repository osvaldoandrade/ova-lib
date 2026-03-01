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

void test_heap_put_with_handle() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    int value = 42;
    void *handle = h->put_with_handle(h, &value);
    print_test_result(handle != NULL && h->size(h) == 1, "Put with handle returns valid handle");
    h->free(h);
}

void test_heap_decrease_key() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    static int values[] = {50, 30, 20, 10};
    void *handles[4];
    
    // Insert values and get handles
    for (int i = 0; i < 4; i++) {
        handles[i] = h->put_with_handle(h, &values[i]);
    }
    
    // Note: decrease_key increases priority. For this max-heap (where larger
    // values have higher priority), we increase the value from 10 to 100
    static int new_val = 100;
    h->decrease_key(h, handles[3], &new_val);
    
    // The maximum should now be 100
    int *max = (int *)h->peek(h);
    print_test_result(*max == 100, "Decrease key updates minimum correctly");
    h->free(h);
}

void test_heap_decrease_key_cascading() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    static int values[10];
    void *handles[10];
    
    // Insert values in descending order to create a specific structure
    for (int i = 0; i < 10; i++) {
        values[i] = 100 - i * 10;  // 100, 90, 80, ...
        handles[i] = h->put_with_handle(h, &values[i]);
    }
    
    // Extract max to trigger consolidation
    h->pop(h);
    
    // Note: decrease_key increases priority. For this max-heap, increase value to 200
    static int new_val = 200;
    h->decrease_key(h, handles[5], &new_val);
    
    // The maximum should now be 200
    int *max = (int *)h->peek(h);
    print_test_result(*max == 200, "Decrease key with cascading cuts works correctly");
    h->free(h);
}

void test_heap_delete_node() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    static int values[] = {50, 30, 20, 10, 40};
    void *handles[5];
    
    // Insert values and get handles
    for (int i = 0; i < 5; i++) {
        handles[i] = h->put_with_handle(h, &values[i]);
    }
    
    // Delete the node with value 30
    h->delete_node(h, handles[1]);
    
    // Size should be 4
    print_test_result(h->size(h) == 4, "Delete node reduces size correctly");
    
    // Extract all elements and verify 30 is not present
    int found_30 = 0;
    while (h->size(h) > 0) {
        int *val = (int *)h->pop(h);
        if (*val == 30) found_30 = 1;
    }
    print_test_result(found_30 == 0, "Deleted node is not in heap");
    h->free(h);
}

void test_heap_delete_min_node() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    static int values[] = {50, 30, 20, 10, 40};
    void *handles[5];
    
    // Insert values and get handles
    for (int i = 0; i < 5; i++) {
        handles[i] = h->put_with_handle(h, &values[i]);
    }
    
    // Delete the minimum node (50)
    h->delete_node(h, handles[0]);
    
    // Minimum should now be 40
    int *min = (int *)h->peek(h);
    print_test_result(*min == 40, "Delete min node updates minimum correctly");
    h->free(h);
}

void test_heap_complex_operations() {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    static int values[20];
    void *handles[20];
    
    // Insert 20 values
    for (int i = 0; i < 20; i++) {
        values[i] = i * 5;  // 0, 5, 10, 15, ...
        handles[i] = h->put_with_handle(h, &values[i]);
    }
    
    // Extract some maximums
    h->pop(h);
    h->pop(h);
    h->pop(h);
    
    // Note: decrease_key increases priority. For max-heap, increase values
    static int inc_vals[3] = {100, 110, 120};
    h->decrease_key(h, handles[10], &inc_vals[0]);
    h->decrease_key(h, handles[5], &inc_vals[1]);
    h->decrease_key(h, handles[2], &inc_vals[2]);
    
    // Delete some nodes
    h->delete_node(h, handles[8]);
    h->delete_node(h, handles[12]);
    
    // Verify maximum is correct (should be 120)
    int *max = (int *)h->peek(h);
    print_test_result(*max == 120, "Complex operations maintain heap property");
    
    // Verify final size (20 - 3 extracted - 2 deleted = 15)
    print_test_result(h->size(h) == 15, "Complex operations maintain correct size");
    h->free(h);
}

void run_all_heap_tests() {
    test_heap_insert_and_extract_max();
    test_heap_peek_max();
    test_heap_empty_check();
    test_heap_pop_empty();
    test_heap_high_volume();
    test_heap_put_with_handle();
    test_heap_decrease_key();
    test_heap_decrease_key_cascading();
    test_heap_delete_node();
    test_heap_delete_min_node();
    test_heap_complex_operations();
}

int main() {
    run_all_heap_tests();
    return 0;
}
