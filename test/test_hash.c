#include "base_test.h"
#include "../include/map.h"
#include <string.h>

#define NUM_THREADS 10
#define OPERATIONS_PER_THREAD 1000  // trimmed to keep test runtime reasonable

int string_compare(const void *a, const void *b) {
    const char *str1 = (const char *)a;
    const char *str2 = (const char *)b;
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL) return -1;
    if (str2 == NULL) return 1;
    return strcmp(str1, str2);
}

void test_insert_and_retrieve_single_item() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key1 = generate_random_string_data();
    char *data1 = "Data1";
    ht->put(ht, key1, data1);
    char *retrieved_data1 = (char *)ht->get(ht, key1);
    print_test_result(strcmp(retrieved_data1, data1) == 0, "Retrieve inserted data");
    free(key1);
    ht->free(ht);
}

void test_check_resizing() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    for (int i = 0; i < 20; i++) {
        char *key = generate_random_string_data();
        ht->put(ht, key, key);
        free(key);
    }
    print_test_result(ht->capacity > 10, "Check resizing (capacity should be greater than initial)");
    ht->free(ht);
}

void test_collision_and_chaining() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key2 = generate_random_string_data();
    char *key3 = generate_random_string_data();
    while (ht->hash_func(key2, ht->capacity) != ht->hash_func(key3, ht->capacity)) {
        free(key3);
        key3 = generate_random_string_data();  // Ensure they collide
    }
    ht->put(ht, key2, "Data2");
    ht->put(ht, key3, "Data3");
    char *retrieved_data2 = (char *)ht->get(ht, key2);
    char *retrieved_data3 = (char *)ht->get(ht, key3);
    print_test_result(strcmp(retrieved_data2, "Data2") == 0 && strcmp(retrieved_data3, "Data3") == 0, "Handle collisions correctly");
    free(key2);
    free(key3);
    ht->free(ht);
}

void test_retrieve_non_existent_item() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *non_existent_key = "non_existent_key";
    char *retrieved_non_existent_data = (char *)ht->get(ht, non_existent_key);
    print_test_result(retrieved_non_existent_data == NULL, "Retrieve non-existent item");
    ht->free(ht);
}

void test_insert_and_retrieve_numeric_keys() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    int numeric_key = 123;
    char *numeric_data = "NumericData";
    ht->put(ht, &numeric_key, numeric_data);
    char *retrieved_numeric_data = (char *)ht->get(ht, &numeric_key);
    print_test_result(strcmp(retrieved_numeric_data, numeric_data) == 0, "Insert and retrieve items with numeric keys");
    ht->free(ht);
}

void test_remove_item() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key_to_remove = "key_to_remove";
    char *data_to_remove = "DataToRemove";
    ht->put(ht, key_to_remove, data_to_remove);
    ht->remove(ht, key_to_remove);
    char *removed_data = (char *)ht->get(ht, key_to_remove);
    print_test_result(removed_data == NULL, "Remove an item");
    ht->free(ht);
}

void test_retrieve_after_removal() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key = "key";
    char *data = "data";
    ht->put(ht, key, data);
    ht->remove(ht, key);
    char *data_after_removal = (char *)ht->get(ht, key);
    print_test_result(data_after_removal == NULL, "Data should be NULL after removal");
    ht->free(ht);
}

void test_insert_retrieve_large_number_of_items() {
    map *ht = create_map(HASH_MAP, 20, NULL, string_compare);
    const int num_items = 15;
    char key[32];
    char data[20];

    for (int i = 0; i < num_items; i++) {
        sprintf(key, "key%d", i);
        sprintf(data, "data%d", i);
        char *stored_key = strdup(key);
        ht->put(ht, stored_key, strdup(data));
        char *retrieved_data = (char *)ht->get(ht, stored_key);
        assert(strcmp(retrieved_data, data) == 0);
    }

    print_test_result(ht->size == num_items, "Correct number of items stored");
    ht->free(ht);
}

void test_handling_of_duplicate_keys() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key = "duplicate_key";
    char *first_data = "first_data";
    char *second_data = "second_data";
    ht->put(ht, key, first_data);
    ht->put(ht, key, second_data);
    char *retrieved_data = (char *)ht->get(ht, key);
    print_test_result(strcmp(retrieved_data, second_data) == 0, "Should retrieve the last inserted data for a duplicate key");
    ht->free(ht);
}

void test_null_key_insertion() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *data = "data_for_null_key";
    ht->put(ht, NULL, data);
    char *retrieved_data = (char *)ht->get(ht, NULL);
    print_test_result(strcmp(retrieved_data, data) == 0, "Should be able to retrieve data inserted with a NULL key");
    ht->free(ht);
}

void test_insert_retrieve_with_null_values() {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key = "test_key";
    ht->put(ht, key, NULL);
    char *retrieved_data = (char *)ht->get(ht, key);
    print_test_result(retrieved_data == NULL, "Should retrieve NULL for inserted NULL value");
    ht->free(ht);
}

void test_map_get_empty() {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    print_test_result(m->get(m, "nope") == NULL, "Get on empty map returns NULL");
    m->free(m);
}

void test_with_high_volume () {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    int num_operations = 10000; // trimmed for faster execution
    char *key = "key";
    char *data = "data";

    for (int i = 0; i < num_operations; i++) {
        ht->put(ht, key, data);
        ht->remove(ht, key);
    }
    print_test_result(ht->size == 0, "Hash table should be empty after repeated insertions and removals");
    ht->free(ht);
}

typedef struct {
    map *ht;
    int id;
} thread_arg;

void *pthread_test_function(void *arg) {
    thread_arg *data = (thread_arg *)arg;
    char key[32];
    char value[32];
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        sprintf(key, "key_%d_%d", data->id, i);
        sprintf(value, "value_%d_%d", data->id, i);
        data->ht->put(data->ht, strdup(key), strdup(value));
        char *retrieved = (char *)data->ht->get(data->ht, key);
        assert_not_null(retrieved);
        assert_string_equal(value, retrieved);
    }
    return NULL;
}

void test_concurrent_access() {
    map *ht = create_map(HASH_TABLE, 50, bernstein_hash, string_compare);  // Create a thread-safe map table
    pthread_t threads[NUM_THREADS];
    thread_arg args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].ht = ht;
        args[i].id = i;
        if (pthread_create(&threads[i], NULL, pthread_test_function, &args[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    ht->free(ht);
    print_test_result(1, "Concurrent access test completed successfully.");
}

void run_all_tests() {
    test_insert_and_retrieve_single_item();
    test_check_resizing();
    test_collision_and_chaining();
    test_retrieve_non_existent_item();
    test_insert_and_retrieve_numeric_keys();
    test_remove_item();
    test_retrieve_after_removal();
    test_insert_retrieve_large_number_of_items();
    test_handling_of_duplicate_keys();
    test_null_key_insertion();
    test_map_get_empty();
    test_insert_retrieve_with_null_values();
    test_with_high_volume();
    test_concurrent_access();
}

int main() {
    run_all_tests();
    return 0;
}