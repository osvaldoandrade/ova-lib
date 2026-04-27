#include "base_test.h"
#include "../include/memory_pool.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/*  Parameter validation                                               */
/* ------------------------------------------------------------------ */

static void test_create_rejects_zero_block_size(void) {
    memory_pool *pool = create_memory_pool(0, 10);
    print_test_result(pool == NULL,
                      "create_memory_pool rejects block_size == 0");
}

static void test_create_rejects_zero_initial_blocks(void) {
    memory_pool *pool = create_memory_pool(64, 0);
    print_test_result(pool == NULL,
                      "create_memory_pool rejects initial_blocks == 0");
}

static void test_create_rejects_negative_initial_blocks(void) {
    memory_pool *pool = create_memory_pool(64, -1);
    print_test_result(pool == NULL,
                      "create_memory_pool rejects initial_blocks < 0");
}

/* ------------------------------------------------------------------ */
/*  Basic allocation and deallocation                                  */
/* ------------------------------------------------------------------ */

static void test_alloc_returns_non_null(void) {
    memory_pool *pool = create_memory_pool(64, 4);
    assert_not_null(pool);

    void *p = memory_pool_alloc(pool);
    print_test_result(p != NULL,
                      "memory_pool_alloc returns non-NULL pointer");

    memory_pool_free(pool, p);
    memory_pool_destroy(pool);
}

static void test_alloc_returns_zeroed_memory(void) {
    memory_pool *pool = create_memory_pool(32, 4);
    assert_not_null(pool);

    void *p = memory_pool_alloc(pool);
    assert_not_null(p);

    unsigned char zero[32];
    memset(zero, 0, sizeof(zero));
    int ok = memcmp(p, zero, sizeof(zero)) == 0;
    print_test_result(ok, "memory_pool_alloc returns zeroed memory");

    memory_pool_free(pool, p);
    memory_pool_destroy(pool);
}

static void test_alloc_multiple_blocks(void) {
    const int n = 8;
    memory_pool *pool = create_memory_pool(sizeof(int), n);
    assert_not_null(pool);

    void *ptrs[8];
    int all_ok = 1;
    for (int i = 0; i < n; i++) {
        ptrs[i] = memory_pool_alloc(pool);
        if (!ptrs[i]) {
            all_ok = 0;
        }
    }
    print_test_result(all_ok, "memory_pool_alloc succeeds for all initial blocks");

    for (int i = 0; i < n; i++) {
        memory_pool_free(pool, ptrs[i]);
    }
    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Recycling                                                          */
/* ------------------------------------------------------------------ */

static void test_free_and_realloc(void) {
    memory_pool *pool = create_memory_pool(64, 2);
    assert_not_null(pool);

    void *a = memory_pool_alloc(pool);
    void *b = memory_pool_alloc(pool);
    assert_not_null(a);
    assert_not_null(b);

    memory_pool_free(pool, a);
    void *c = memory_pool_alloc(pool);

    /* The recycled block should be reused (same address as a). */
    print_test_result(c == a, "memory_pool recycles freed blocks");

    memory_pool_free(pool, b);
    memory_pool_free(pool, c);
    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Auto-grow                                                          */
/* ------------------------------------------------------------------ */

static void test_pool_grows_automatically(void) {
    memory_pool *pool = create_memory_pool(16, 2);
    assert_not_null(pool);

    void *a = memory_pool_alloc(pool);
    void *b = memory_pool_alloc(pool);
    assert_not_null(a);
    assert_not_null(b);

    /* Third allocation should trigger a new chunk. */
    void *c = memory_pool_alloc(pool);
    print_test_result(c != NULL,
                      "memory_pool grows when initial blocks exhausted");

    memory_pool_free(pool, a);
    memory_pool_free(pool, b);
    memory_pool_free(pool, c);
    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Active count tracking                                              */
/* ------------------------------------------------------------------ */

static void test_active_count(void) {
    memory_pool *pool = create_memory_pool(32, 4);
    assert_not_null(pool);

    print_test_result(memory_pool_active_count(pool) == 0,
                      "active count is 0 on creation");

    void *a = memory_pool_alloc(pool);
    void *b = memory_pool_alloc(pool);
    print_test_result(memory_pool_active_count(pool) == 2,
                      "active count is 2 after two allocations");

    memory_pool_free(pool, a);
    print_test_result(memory_pool_active_count(pool) == 1,
                      "active count is 1 after freeing one block");

    memory_pool_free(pool, b);
    print_test_result(memory_pool_active_count(pool) == 0,
                      "active count is 0 after freeing all blocks");

    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  NULL safety                                                        */
/* ------------------------------------------------------------------ */

static void test_null_safety(void) {
    /* All functions must accept NULL without crashing. */
    void *p = memory_pool_alloc(NULL);
    print_test_result(p == NULL,
                      "memory_pool_alloc(NULL) returns NULL");

    memory_pool_free(NULL, (void *)0x1); /* should not crash */
    print_test_result(1, "memory_pool_free(NULL, ptr) is safe");

    memory_pool *pool = create_memory_pool(16, 4);
    assert_not_null(pool);
    memory_pool_free(pool, NULL); /* should not crash */
    print_test_result(1, "memory_pool_free(pool, NULL) is safe");
    memory_pool_destroy(pool);

    print_test_result(memory_pool_active_count(NULL) == 0,
                      "memory_pool_active_count(NULL) returns 0");

    memory_pool_destroy(NULL); /* should not crash */
    print_test_result(1, "memory_pool_destroy(NULL) is safe");
}

/* ------------------------------------------------------------------ */
/*  Small block size (smaller than pointer)                            */
/* ------------------------------------------------------------------ */

static void test_small_block_size(void) {
    /* block_size == 1 is smaller than sizeof(void*), should still work. */
    memory_pool *pool = create_memory_pool(1, 8);
    assert_not_null(pool);

    void *a = memory_pool_alloc(pool);
    void *b = memory_pool_alloc(pool);
    print_test_result(a != NULL && b != NULL,
                      "memory_pool handles block_size < sizeof(void*)");

    memory_pool_free(pool, a);
    memory_pool_free(pool, b);
    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Write / read back                                                  */
/* ------------------------------------------------------------------ */

static void test_write_and_read(void) {
    memory_pool *pool = create_memory_pool(sizeof(int), 4);
    assert_not_null(pool);

    int *a = (int *)memory_pool_alloc(pool);
    int *b = (int *)memory_pool_alloc(pool);
    assert_not_null(a);
    assert_not_null(b);

    *a = 42;
    *b = 99;

    int ok = (*a == 42) && (*b == 99);
    print_test_result(ok, "memory_pool blocks are writable and readable");

    memory_pool_free(pool, a);
    memory_pool_free(pool, b);
    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Large number of allocations                                        */
/* ------------------------------------------------------------------ */

static void test_many_allocations(void) {
    const int n = 1000;
    memory_pool *pool = create_memory_pool(sizeof(int), 16);
    assert_not_null(pool);

    void *ptrs[1000];
    int all_ok = 1;
    for (int i = 0; i < n; i++) {
        ptrs[i] = memory_pool_alloc(pool);
        if (!ptrs[i]) {
            all_ok = 0;
        }
    }
    print_test_result(all_ok,
                      "memory_pool handles 1000 allocations (auto-grow)");
    print_test_result(memory_pool_active_count(pool) == 1000,
                      "active count is 1000 after 1000 allocations");

    for (int i = 0; i < n; i++) {
        memory_pool_free(pool, ptrs[i]);
    }
    print_test_result(memory_pool_active_count(pool) == 0,
                      "active count is 0 after freeing all 1000 blocks");

    memory_pool_destroy(pool);
}

/* ------------------------------------------------------------------ */
/*  Runner                                                             */
/* ------------------------------------------------------------------ */

static void run_all_tests(void) {
    test_create_rejects_zero_block_size();
    test_create_rejects_zero_initial_blocks();
    test_create_rejects_negative_initial_blocks();
    test_alloc_returns_non_null();
    test_alloc_returns_zeroed_memory();
    test_alloc_multiple_blocks();
    test_free_and_realloc();
    test_pool_grows_automatically();
    test_active_count();
    test_null_safety();
    test_small_block_size();
    test_write_and_read();
    test_many_allocations();
}

int main(void) {
    run_all_tests();
    return 0;
}
