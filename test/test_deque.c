#include "base_test.h"
#include "../include/deque.h"
#include "../src/utils/capacity_utils.h"
#include <limits.h>
#include <string.h>
#include <time.h>

void test_deque_create(void) {
    deque *d = create_deque(10);
    print_test_result(d != NULL, "Deque should be created successfully");
    print_test_result(deque_is_empty(d), "New deque should be empty");
    print_test_result(deque_size(d) == 0, "New deque should have size 0");
    deque_free(d);
}

void test_deque_create_default_capacity(void) {
    deque *d = create_deque(0);
    print_test_result(d != NULL, "Deque with default capacity should be created");
    deque_free(d);
}

void test_deque_push_front_single(void) {
    deque *d = create_deque(10);
    int value = 42;
    deque_push_front(d, &value);
    print_test_result(!deque_is_empty(d), "Deque should not be empty after push_front");
    print_test_result(deque_size(d) == 1, "Deque size should be 1 after one push_front");
    int *result = (int*)deque_peek_front(d);
    print_test_result(result != NULL && *result == 42, "Front element should be 42");
    deque_free(d);
}

void test_deque_push_back_single(void) {
    deque *d = create_deque(10);
    int value = 99;
    deque_push_back(d, &value);
    print_test_result(!deque_is_empty(d), "Deque should not be empty after push_back");
    print_test_result(deque_size(d) == 1, "Deque size should be 1 after one push_back");
    int *result = (int*)deque_peek_back(d);
    print_test_result(result != NULL && *result == 99, "Back element should be 99");
    deque_free(d);
}

void test_deque_push_pop_front(void) {
    deque *d = create_deque(10);
    int value1 = 1, value2 = 2, value3 = 3;
    
    deque_push_front(d, &value1);
    deque_push_front(d, &value2);
    deque_push_front(d, &value3);
    
    print_test_result(deque_size(d) == 3, "Deque size should be 3 after three push_front");
    
    int *result = (int*)deque_pop_front(d);
    print_test_result(result != NULL && *result == 3, "First pop_front should return 3");
    
    result = (int*)deque_pop_front(d);
    print_test_result(result != NULL && *result == 2, "Second pop_front should return 2");
    
    result = (int*)deque_pop_front(d);
    print_test_result(result != NULL && *result == 1, "Third pop_front should return 1");
    
    print_test_result(deque_is_empty(d), "Deque should be empty after popping all elements");
    deque_free(d);
}

void test_deque_push_pop_back(void) {
    deque *d = create_deque(10);
    int value1 = 10, value2 = 20, value3 = 30;
    
    deque_push_back(d, &value1);
    deque_push_back(d, &value2);
    deque_push_back(d, &value3);
    
    print_test_result(deque_size(d) == 3, "Deque size should be 3 after three push_back");
    
    int *result = (int*)deque_pop_back(d);
    print_test_result(result != NULL && *result == 30, "First pop_back should return 30");
    
    result = (int*)deque_pop_back(d);
    print_test_result(result != NULL && *result == 20, "Second pop_back should return 20");
    
    result = (int*)deque_pop_back(d);
    print_test_result(result != NULL && *result == 10, "Third pop_back should return 10");
    
    print_test_result(deque_is_empty(d), "Deque should be empty after popping all elements");
    deque_free(d);
}

void test_deque_mixed_operations(void) {
    deque *d = create_deque(10);
    int values[6] = {1, 2, 3, 4, 5, 6};
    
    deque_push_back(d, &values[0]);   // [1]
    deque_push_front(d, &values[1]);  // [2, 1]
    deque_push_back(d, &values[2]);   // [2, 1, 3]
    deque_push_front(d, &values[3]);  // [4, 2, 1, 3]
    
    print_test_result(deque_size(d) == 4, "Deque size should be 4");
    
    int *result = (int*)deque_peek_front(d);
    print_test_result(result != NULL && *result == 4, "Front should be 4");
    
    result = (int*)deque_peek_back(d);
    print_test_result(result != NULL && *result == 3, "Back should be 3");
    
    result = (int*)deque_pop_front(d);  // [2, 1, 3]
    print_test_result(result != NULL && *result == 4, "Pop front should return 4");
    
    result = (int*)deque_pop_back(d);   // [2, 1]
    print_test_result(result != NULL && *result == 3, "Pop back should return 3");
    
    print_test_result(deque_size(d) == 2, "Deque size should be 2");
    
    deque_free(d);
}

void test_deque_get(void) {
    deque *d = create_deque(10);
    int values[5] = {10, 20, 30, 40, 50};
    
    for (int i = 0; i < 5; i++) {
        deque_push_back(d, &values[i]);
    }
    
    print_test_result(deque_size(d) == 5, "Deque size should be 5");
    
    for (int i = 0; i < 5; i++) {
        int *result = (int*)deque_get(d, i);
        int expected = (i + 1) * 10;
        print_test_result(result != NULL && *result == expected, 
                         "Element at index should match expected value");
    }
    
    // Test out of bounds
    int *result = (int*)deque_get(d, -1);
    print_test_result(result == NULL, "Get with negative index should return NULL");
    
    result = (int*)deque_get(d, 5);
    print_test_result(result == NULL, "Get with index >= size should return NULL");
    
    deque_free(d);
}

void test_deque_random_access_after_operations(void) {
    deque *d = create_deque(5);
    int values[5] = {1, 2, 3, 4, 5};
    
    // Push to back: [1, 2, 3, 4, 5]
    for (int i = 0; i < 5; i++) {
        deque_push_back(d, &values[i]);
    }
    
    // Pop from front: [2, 3, 4, 5]
    deque_pop_front(d);
    
    // Verify random access
    int *result = (int*)deque_get(d, 0);
    print_test_result(result != NULL && *result == 2, "Index 0 should be 2");
    
    result = (int*)deque_get(d, 3);
    print_test_result(result != NULL && *result == 5, "Index 3 should be 5");
    
    deque_free(d);
}

void test_deque_resize(void) {
    deque *d = create_deque(4);
    int values[20];
    
    // Fill deque beyond initial capacity
    for (int i = 0; i < 20; i++) {
        values[i] = i;
        deque_push_back(d, &values[i]);
    }
    
    print_test_result(deque_size(d) == 20, "Deque should contain 20 elements after resize");
    
    // Verify all elements are accessible
    for (int i = 0; i < 20; i++) {
        int *result = (int*)deque_get(d, i);
        print_test_result(result != NULL && *result == i, "Element should be correct after resize");
    }
    
    deque_free(d);
}

void test_deque_resize_with_front_operations(void) {
    deque *d = create_deque(4);
    int values[20];
    
    // Fill deque beyond initial capacity using push_front
    for (int i = 0; i < 20; i++) {
        values[i] = i;
        deque_push_front(d, &values[i]);
    }
    
    print_test_result(deque_size(d) == 20, "Deque should contain 20 elements");
    
    // Elements should be in reverse order
    for (int i = 0; i < 20; i++) {
        int *result = (int*)deque_get(d, i);
        int expected = 19 - i;
        print_test_result(result != NULL && *result == expected, 
                         "Element should be in correct order after front resize");
    }
    
    deque_free(d);
}

void test_deque_pop_empty(void) {
    deque *d = create_deque(10);
    
    void *result = deque_pop_front(d);
    print_test_result(result == NULL, "Pop front on empty deque should return NULL");
    
    result = deque_pop_back(d);
    print_test_result(result == NULL, "Pop back on empty deque should return NULL");
    
    deque_free(d);
}

void test_deque_peek_empty(void) {
    deque *d = create_deque(10);
    
    void *result = deque_peek_front(d);
    print_test_result(result == NULL, "Peek front on empty deque should return NULL");
    
    result = deque_peek_back(d);
    print_test_result(result == NULL, "Peek back on empty deque should return NULL");
    
    deque_free(d);
}

void test_deque_null_operations(void) {
    print_test_result(deque_size(NULL) == 0, "Size of NULL deque should be 0");
    print_test_result(deque_is_empty(NULL), "NULL deque should be considered empty");
    print_test_result(deque_pop_front(NULL) == NULL, "Pop front on NULL deque should return NULL");
    print_test_result(deque_pop_back(NULL) == NULL, "Pop back on NULL deque should return NULL");
    print_test_result(deque_peek_front(NULL) == NULL, "Peek front on NULL deque should return NULL");
    print_test_result(deque_peek_back(NULL) == NULL, "Peek back on NULL deque should return NULL");
    print_test_result(deque_get(NULL, 0) == NULL, "Get on NULL deque should return NULL");
    
    // These should not crash
    deque_push_front(NULL, NULL);
    deque_push_back(NULL, NULL);
    deque_free(NULL);
    print_test_result(1, "NULL deque operations should not crash");
}

void test_deque_circular_wrap(void) {
    deque *d = create_deque(8);
    int values[10];
    
    // Fill the deque
    for (int i = 0; i < 8; i++) {
        values[i] = i;
        deque_push_back(d, &values[i]);
    }
    
    // Pop some from front to move front pointer
    deque_pop_front(d);
    deque_pop_front(d);
    deque_pop_front(d);
    
    // Push more to back (should wrap around)
    values[8] = 8;
    values[9] = 9;
    deque_push_back(d, &values[8]);
    deque_push_back(d, &values[9]);
    
    // Verify elements are correct
    print_test_result(deque_size(d) == 7, "Deque size should be 7");
    
    int *result = (int*)deque_get(d, 0);
    print_test_result(result != NULL && *result == 3, "First element should be 3");
    
    result = (int*)deque_get(d, 6);
    print_test_result(result != NULL && *result == 9, "Last element should be 9");
    
    deque_free(d);
}

void test_deque_high_volume(void) {
    deque *d = create_deque(16);
    const int max_data = 1000;
    clock_t start = clock();
    int* data_array = generate_random_int_data((size_t)max_data);
    
    // Push all elements to back
    for (int i = 0; i < max_data; i++) {
        deque_push_back(d, &data_array[i]);
    }
    
    // Verify all elements via random access
    for (int i = 0; i < max_data; i++) {
        int *result = (int*)deque_get(d, i);
        assert(result != NULL && *result == data_array[i]);
        (void)result;
    }
    
    // Pop all elements from front
    for (int i = 0; i < max_data; i++) {
        int *result = (int*)deque_pop_front(d);
        assert(result != NULL && *result == data_array[i]);
        (void)result;
    }
    
    assert(deque_is_empty(d));
    
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1000.0, "Deque high volume operations within time limit");
    
    free(data_array);
    deque_free(d);
}

void test_deque_with_string_data(void) {
    deque *d = create_deque(10);
    char* str1 = generate_random_string_data();
    char* str2 = generate_random_string_data();
    
    deque_push_front(d, str1);
    deque_push_back(d, str2);
    
    char* result = (char*)deque_peek_front(d);
    print_test_result(result != NULL && strcmp(result, str1) == 0, 
                     "Front string should match");
    
    result = (char*)deque_peek_back(d);
    print_test_result(result != NULL && strcmp(result, str2) == 0, 
                     "Back string should match");
    
    free(str1);
    free(str2);
    deque_free(d);
}

void test_deque_alternating_operations(void) {
    deque *d = create_deque(10);
    int values[10];
    
    // Alternating push front and back
    for (int i = 0; i < 10; i++) {
        values[i] = i;
        if (i % 2 == 0) {
            deque_push_front(d, &values[i]);
        } else {
            deque_push_back(d, &values[i]);
        }
    }
    
    print_test_result(deque_size(d) == 10, "Deque should have 10 elements");
    
    // Alternating pop front and back
    int count = 0;
    while (!deque_is_empty(d)) {
        if (count % 2 == 0) {
            int *result = (int*)deque_pop_front(d);
            print_test_result(result != NULL, "Pop front should return non-NULL");
        } else {
            int *result = (int*)deque_pop_back(d);
            print_test_result(result != NULL, "Pop back should return non-NULL");
        }
        count++;
    }
    
    print_test_result(deque_is_empty(d), "Deque should be empty after all pops");
    deque_free(d);
}

void test_safe_double_capacity_for_deque(void) {
    print_test_result(safe_double_capacity(16) == 32, "Deque safe_double_capacity(16) == 32");
    print_test_result(safe_double_capacity(INT_MAX / 2 + 1) == INT_MAX, "Deque safe_double_capacity caps at INT_MAX");
    print_test_result(safe_double_capacity(INT_MAX) == INT_MAX, "Deque safe_double_capacity(INT_MAX) stays INT_MAX");
}

void run_all_deque_tests(void) {
    test_safe_double_capacity_for_deque();
    test_deque_create();
    test_deque_create_default_capacity();
    test_deque_push_front_single();
    test_deque_push_back_single();
    test_deque_push_pop_front();
    test_deque_push_pop_back();
    test_deque_mixed_operations();
    test_deque_get();
    test_deque_random_access_after_operations();
    test_deque_resize();
    test_deque_resize_with_front_operations();
    test_deque_pop_empty();
    test_deque_peek_empty();
    test_deque_null_operations();
    test_deque_circular_wrap();
    test_deque_high_volume();
    test_deque_with_string_data();
    test_deque_alternating_operations();
}

int main(void) {
    run_all_deque_tests();
    return 0;
}
