#include "base_test.h"
#include "../include/map.h"
#include "../src/utils/capacity_utils.h"
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 10
#define OPERATIONS_PER_THREAD 5000

int string_compare(const void *a, const void *b) {
    const char *str1 = (const char *)a;
    const char *str2 = (const char *)b;
    if (str1 == NULL && str2 == NULL) return 0;
    if (str1 == NULL) return -1;
    if (str2 == NULL) return 1;
    return strcmp(str1, str2);
}

static int int_compare(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int int_hash(void *key, int capacity) {
    if (!key || capacity <= 0) {
        return 0;
    }
    return (*(int *)key) % capacity;
}

static int constant_hash(void *key, int capacity) {
    (void)key;
    (void)capacity;
    return 0;
}

void test_insert_and_retrieve_single_item(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *key1 = generate_random_string_data();
    char data1[] = "Data1";
    ht->put(ht, key1, data1);
    char *retrieved_data1 = (char *)ht->get(ht, key1);
    print_test_result(strcmp(retrieved_data1, data1) == 0, "Retrieve inserted data");
    ht->free(ht);
    free(key1);
}

void test_check_resizing(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char *keys[20];
    for (int i = 0; i < 20; i++) {
        keys[i] = generate_random_string_data();
        ht->put(ht, keys[i], keys[i]);
    }
    print_test_result(ht->capacity(ht) > 10, "Check resizing (capacity should be greater than initial)");
    ht->free(ht);
    for (int i = 0; i < 20; i++) {
        free(keys[i]);
    }
}

void test_collision_and_chaining(void) {
    map *ht = create_map(HASH_MAP, 10, constant_hash, string_compare);
    char *key2 = generate_random_string_data();
    char *key3 = generate_random_string_data();
    while (strcmp(key2, key3) == 0) {
        free(key3);
        key3 = generate_random_string_data();
    }
    char data2[] = "Data2";
    char data3[] = "Data3";
    ht->put(ht, key2, data2);
    ht->put(ht, key3, data3);
    char *retrieved_data2 = (char *)ht->get(ht, key2);
    char *retrieved_data3 = (char *)ht->get(ht, key3);
    print_test_result(strcmp(retrieved_data2, "Data2") == 0 && strcmp(retrieved_data3, "Data3") == 0, "Handle collisions correctly");
    ht->free(ht);
    free(key2);
    free(key3);
}

void test_retrieve_non_existent_item(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char non_existent_key[] = "non_existent_key";
    char *retrieved_non_existent_data = (char *)ht->get(ht, non_existent_key);
    print_test_result(retrieved_non_existent_data == NULL, "Retrieve non-existent item");
    ht->free(ht);
}

void test_insert_and_retrieve_numeric_keys(void) {
    map *ht = create_map(HASH_MAP, 10, int_hash, int_compare);
    int numeric_key = 123;
    char numeric_data[] = "NumericData";
    ht->put(ht, &numeric_key, numeric_data);
    char *retrieved_numeric_data = (char *)ht->get(ht, &numeric_key);
    print_test_result(strcmp(retrieved_numeric_data, numeric_data) == 0, "Insert and retrieve items with numeric keys");
    ht->free(ht);
}

void test_remove_item(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char key_to_remove[] = "key_to_remove";
    char data_to_remove[] = "DataToRemove";
    ht->put(ht, key_to_remove, data_to_remove);
    ht->remove(ht, key_to_remove);
    char *removed_data = (char *)ht->get(ht, key_to_remove);
    print_test_result(removed_data == NULL, "Remove an item");
    ht->free(ht);
}

void test_retrieve_after_removal(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char key[] = "key";
    char data[] = "data";
    ht->put(ht, key, data);
    ht->remove(ht, key);
    char *data_after_removal = (char *)ht->get(ht, key);
    print_test_result(data_after_removal == NULL, "Data should be NULL after removal");
    ht->free(ht);
}

void test_insert_retrieve_large_number_of_items(void) {
    map *ht = create_map(HASH_MAP, 20, NULL, string_compare);
    const int num_items = 15;
    char keys[15][32];
    char data[15][20];

    for (int i = 0; i < num_items; i++) {
        sprintf(keys[i], "key%d", i);
        sprintf(data[i], "data%d", i);
        ht->put(ht, keys[i], data[i]);
        char *retrieved_data = (char *)ht->get(ht, keys[i]);
        assert_not_null(retrieved_data);
        assert_string_equal(data[i], retrieved_data);
    }

    print_test_result(ht->size(ht) == num_items, "Correct number of items stored");
    ht->free(ht);
}

void test_handling_of_duplicate_keys(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char key[] = "duplicate_key";
    char first_data[] = "first_data";
    char second_data[] = "second_data";
    ht->put(ht, key, first_data);
    ht->put(ht, key, second_data);
    char *retrieved_data = (char *)ht->get(ht, key);
    print_test_result(strcmp(retrieved_data, second_data) == 0, "Should retrieve the last inserted data for a duplicate key");
    ht->free(ht);
}

void test_null_key_insertion(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char data[] = "data_for_null_key";
    ht->put(ht, NULL, data);
    char *retrieved_data = (char *)ht->get(ht, NULL);
    print_test_result(strcmp(retrieved_data, data) == 0, "Should be able to retrieve data inserted with a NULL key");
    ht->free(ht);
}

static int unsafe_string_compare(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

void test_null_key_with_unsafe_comparator(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, unsafe_string_compare);
    char data[] = "data_for_null_key";
    ht->put(ht, NULL, data);
    char *retrieved_data = (char *)ht->get(ht, NULL);
    print_test_result(retrieved_data != NULL && strcmp(retrieved_data, data) == 0,
                      "NULL key put/get with unsafe comparator should not segfault");
    ht->free(ht);
}

void test_null_key_lookup_with_unsafe_comparator(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, unsafe_string_compare);
    char key[] = "real_key";
    char data[] = "data";
    ht->put(ht, key, data);
    char *result = (char *)ht->get(ht, NULL);
    print_test_result(result == NULL, "Get with NULL key should return NULL when no NULL key stored");
    ht->free(ht);
}

void test_null_key_remove_with_unsafe_comparator(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, unsafe_string_compare);
    char data[] = "data_for_null";
    ht->put(ht, NULL, data);
    char *removed = (char *)ht->remove(ht, NULL);
    print_test_result(removed != NULL && strcmp(removed, data) == 0,
                      "Remove with NULL key should work with unsafe comparator");
    char *after = (char *)ht->get(ht, NULL);
    print_test_result(after == NULL, "Get after NULL key removal returns NULL");
    ht->free(ht);
}

void test_null_and_non_null_keys_coexist(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, unsafe_string_compare);
    char data_null[] = "null_data";
    char key[] = "real_key";
    char data_key[] = "key_data";
    ht->put(ht, NULL, data_null);
    ht->put(ht, key, data_key);
    char *r1 = (char *)ht->get(ht, NULL);
    char *r2 = (char *)ht->get(ht, key);
    print_test_result(r1 != NULL && strcmp(r1, data_null) == 0,
                      "NULL key data retrievable alongside non-NULL keys");
    print_test_result(r2 != NULL && strcmp(r2, data_key) == 0,
                      "Non-NULL key data retrievable alongside NULL key");
    ht->free(ht);
}

void test_insert_retrieve_with_null_values(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    char key[] = "test_key";
    ht->put(ht, key, NULL);
    char *retrieved_data = (char *)ht->get(ht, key);
    print_test_result(retrieved_data == NULL, "Should retrieve NULL for inserted NULL value");
    ht->free(ht);
}

void test_map_get_empty(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    char missing[] = "missing";
    print_test_result(m->get(m, missing) == NULL, "Get on empty map returns NULL");
    m->free(m);
}

void test_with_high_volume(void) {
    map *ht = create_map(HASH_MAP, 10, NULL, string_compare);
    int num_operations = 20000;
    clock_t start = clock();
    char key[] = "key";
    char data[] = "data";

    for (int i = 0; i < num_operations; i++) {
        ht->put(ht, key, data);
        ht->remove(ht, key);
    }
    print_test_result(ht->size(ht) == 0, "Hash table should be empty after repeated insertions and removals");
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1500.0, "Hash map high volume within time limit");
    ht->free(ht);
}

typedef struct {
    map *ht;
    int id;
    char (*keys)[32];
    char (*values)[32];
} thread_arg;

void *pthread_test_function(void *arg) {
    thread_arg *data = (thread_arg *)arg;
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        sprintf(data->keys[i], "key_%d_%d", data->id, i);
        sprintf(data->values[i], "value_%d_%d", data->id, i);
        data->ht->put(data->ht, data->keys[i], data->values[i]);
        char *retrieved = (char *)data->ht->get(data->ht, data->keys[i]);
        assert_not_null(retrieved);
        assert_string_equal(data->values[i], retrieved);
    }
    return NULL;
}

void test_concurrent_access(void) {
    map *ht = create_map(HASH_TABLE, 50, bernstein_hash, string_compare);  // Create a thread-safe map table
    pthread_t threads[NUM_THREADS];
    thread_arg args[NUM_THREADS] = {0};

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].ht = ht;
        args[i].id = i;
        args[i].keys = calloc(OPERATIONS_PER_THREAD, sizeof(*args[i].keys));
        args[i].values = calloc(OPERATIONS_PER_THREAD, sizeof(*args[i].values));
        if (args[i].keys == NULL || args[i].values == NULL) {
            perror("Failed to allocate thread buffers");
            free(args[i].keys);
            free(args[i].values);
            ht->free(ht);
            for (int j = 0; j < i; j++) {
                free(args[j].keys);
                free(args[j].values);
            }
            return;
        }
        if (pthread_create(&threads[i], NULL, pthread_test_function, &args[i]) != 0) {
            perror("Failed to create thread");
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    ht->free(ht);
    for (int i = 0; i < NUM_THREADS; i++) {
        free(args[i].keys);
        free(args[i].values);
    }
    print_test_result(1, "Concurrent access test completed successfully.");
}

void test_map_put_bulk(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    char k1[] = "key1", k2[] = "key2", k3[] = "key3";
    char v1[] = "val1", v2[] = "val2", v3[] = "val3";
    void *keys[] = {k1, k2, k3};
    void *vals[] = {v1, v2, v3};

    m->put_bulk(m, keys, vals, 3);
    print_test_result(m->size(m) == 3, "Map bulk put inserts all pairs");

    int ok = 1;
    for (int i = 0; i < 3; i++) {
        char *retrieved = (char *)m->get(m, keys[i]);
        if (!retrieved || strcmp(retrieved, (char *)vals[i]) != 0) {
            ok = 0;
        }
    }
    print_test_result(ok, "Map bulk put values are all retrievable");

    m->free(m);
}

void test_map_put_bulk_with_duplicate_keys(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    char k1[] = "key1", k2[] = "key2", k3[] = "key1";
    char v1[] = "val1", v2[] = "val2", v3[] = "val3";
    void *keys[] = {k1, k2, k3};
    void *vals[] = {v1, v2, v3};

    m->put_bulk(m, keys, vals, 3);
    print_test_result(m->size(m) == 2, "Map bulk put with duplicates has correct size");

    char key1_lookup[] = "key1";
    char *retrieved = (char *)m->get(m, key1_lookup);
    print_test_result(retrieved && strcmp(retrieved, "val3") == 0, "Map bulk put duplicate key retains last value");

    m->free(m);
}

void test_map_put_bulk_edge_cases(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    char k1[] = "k";
    char v1[] = "v";
    void *keys[] = {k1};
    void *vals[] = {v1};

    m->put_bulk(m, keys, vals, 0);
    print_test_result(m->size(m) == 0, "Map bulk put with count 0 does nothing");

    m->put_bulk(m, NULL, vals, 3);
    print_test_result(m->size(m) == 0, "Map bulk put with NULL keys does nothing");

    m->put_bulk(m, keys, NULL, 3);
    print_test_result(m->size(m) == 0, "Map bulk put with NULL values does nothing");

    m->put_bulk(NULL, keys, vals, 1);
    print_test_result(1, "Map bulk put with NULL map does not crash");

    m->free(m);
}

void test_safe_double_capacity_for_hash_map(void) {
    print_test_result(safe_double_capacity(10) == 20, "Hash map safe_double_capacity(10) == 20");
    print_test_result(safe_double_capacity(INT_MAX / 2 + 1) == INT_MAX, "Hash map safe_double_capacity caps at INT_MAX");
    print_test_result(safe_double_capacity(INT_MAX) == INT_MAX, "Hash map safe_double_capacity(INT_MAX) stays INT_MAX");
}

void run_all_tests(void) {
    test_safe_double_capacity_for_hash_map();
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
    test_null_key_with_unsafe_comparator();
    test_null_key_lookup_with_unsafe_comparator();
    test_null_key_remove_with_unsafe_comparator();
    test_null_and_non_null_keys_coexist();
    test_insert_retrieve_with_null_values();
    test_map_get_empty();
    test_with_high_volume();
    test_concurrent_access();
    test_map_put_bulk();
    test_map_put_bulk_with_duplicate_keys();
    test_map_put_bulk_edge_cases();
}

int main(void) {
    run_all_tests();
    return 0;
}
