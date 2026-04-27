#include "base_test.h"
#include "../include/list.h"
#include "../include/queue.h"
#include "../include/stack.h"
#include "../include/heap.h"
#include "../include/map.h"
#include <string.h>

/* ── comparators ───────────────────────────────────────────────────── */

static int int_compare(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int string_compare(const void *a, const void *b) {
    const char *s1 = (const char *)a;
    const char *s2 = (const char *)b;
    if (s1 == NULL && s2 == NULL) return 0;
    if (s1 == NULL) return -1;
    if (s2 == NULL) return 1;
    return strcmp(s1, s2);
}

/* ── list clear tests ──────────────────────────────────────────────── */

static void test_array_list_clear(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }
    print_test_result(lst->size(lst) == 5, "Array list has 5 elements before clear");

    lst->clear(lst);
    print_test_result(lst->size(lst) == 0, "Array list is empty after clear");

    /* Verify we can reuse the list */
    int val = 99;
    lst->insert(lst, &val, 0);
    print_test_result(lst->size(lst) == 1, "Array list usable after clear");
    int *retrieved = (int *)lst->get(lst, 0);
    print_test_result(retrieved != NULL && *retrieved == 99, "Array list get after clear returns correct value");

    lst->free(lst);
}

static void test_linked_list_clear(void) {
    list *lst = create_list(LINKED_LIST, 0, NULL);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], i);
    }
    print_test_result(lst->size(lst) == 5, "Linked list has 5 elements before clear");

    lst->clear(lst);
    print_test_result(lst->size(lst) == 0, "Linked list is empty after clear");

    /* Verify we can reuse the list */
    int val = 77;
    lst->insert(lst, &val, 0);
    print_test_result(lst->size(lst) == 1, "Linked list usable after clear");
    int *retrieved = (int *)lst->get(lst, 0);
    print_test_result(retrieved != NULL && *retrieved == 77, "Linked list get after clear returns correct value");

    lst->free(lst);
}

static void test_sorted_list_clear(void) {
    list *lst = create_list(SORTED_LIST, 4, int_compare);
    int items[] = {5, 1, 4, 3, 2};
    for (int i = 0; i < 5; i++) {
        lst->insert(lst, &items[i], 0);
    }
    print_test_result(lst->size(lst) == 5, "Sorted list has 5 elements before clear");

    lst->clear(lst);
    print_test_result(lst->size(lst) == 0, "Sorted list is empty after clear");

    /* Verify we can reuse the list and it maintains sort order */
    int a = 30, b = 10, c = 20;
    lst->insert(lst, &a, 0);
    lst->insert(lst, &b, 0);
    lst->insert(lst, &c, 0);
    print_test_result(lst->size(lst) == 3, "Sorted list usable after clear");
    int *first = (int *)lst->get(lst, 0);
    print_test_result(first != NULL && *first == 10, "Sorted list maintains order after clear");

    lst->free(lst);
}

static void test_list_clear_empty(void) {
    list *lst = create_list(ARRAY_LIST, 4, NULL);
    lst->clear(lst);
    print_test_result(lst->size(lst) == 0, "Clear on empty array list is safe");
    lst->free(lst);

    lst = create_list(LINKED_LIST, 0, NULL);
    lst->clear(lst);
    print_test_result(lst->size(lst) == 0, "Clear on empty linked list is safe");
    lst->free(lst);
}

/* ── queue clear tests ─────────────────────────────────────────────── */

static void test_queue_clear(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        q->enqueue(q, &data[i]);
    }
    print_test_result(q->size(q) == 5, "Queue has 5 elements before clear");

    q->clear(q);
    print_test_result(q->is_empty(q), "Queue is empty after clear");
    print_test_result(q->size(q) == 0, "Queue size is 0 after clear");

    /* Verify we can reuse the queue */
    int val = 42;
    q->enqueue(q, &val);
    print_test_result(q->size(q) == 1, "Queue usable after clear");
    int *dequeued = (int *)q->dequeue(q);
    print_test_result(dequeued != NULL && *dequeued == 42, "Queue dequeue after clear returns correct value");

    q->free(q);
}

static void test_priority_queue_clear(void) {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_compare);
    int data[] = {15, 42, 3, 56, 7};
    for (int i = 0; i < 5; i++) {
        pq->enqueue(pq, &data[i]);
    }
    print_test_result(pq->size(pq) == 5, "Priority queue has 5 elements before clear");

    pq->clear(pq);
    print_test_result(pq->is_empty(pq), "Priority queue is empty after clear");
    print_test_result(pq->size(pq) == 0, "Priority queue size is 0 after clear");

    /* Verify we can reuse the priority queue */
    int val = 99;
    pq->enqueue(pq, &val);
    print_test_result(pq->size(pq) == 1, "Priority queue usable after clear");

    pq->free(pq);
}

static void test_queue_clear_empty(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 4, NULL);
    q->clear(q);
    print_test_result(q->is_empty(q), "Clear on empty queue is safe");
    q->free(q);
}

/* ── stack clear tests ─────────────────────────────────────────────── */

static void test_array_stack_clear(void) {
    stack *stk = create_stack(ARRAY_STACK);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        stk->push(stk, &items[i]);
    }
    print_test_result(stk->size(stk) == 5, "Array stack has 5 elements before clear");

    stk->clear(stk);
    print_test_result(stk->is_empty(stk), "Array stack is empty after clear");
    print_test_result(stk->size(stk) == 0, "Array stack size is 0 after clear");

    /* Verify we can reuse the stack */
    int val = 77;
    stk->push(stk, &val);
    print_test_result(stk->size(stk) == 1, "Array stack usable after clear");
    int *top = (int *)stk->top(stk);
    print_test_result(top != NULL && *top == 77, "Array stack top after clear returns correct value");

    stk->free(stk);
}

static void test_linked_stack_clear(void) {
    stack *stk = create_stack(LINKED_STACK);
    int items[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        stk->push(stk, &items[i]);
    }
    print_test_result(stk->size(stk) == 5, "Linked stack has 5 elements before clear");

    stk->clear(stk);
    print_test_result(stk->is_empty(stk), "Linked stack is empty after clear");
    print_test_result(stk->size(stk) == 0, "Linked stack size is 0 after clear");

    /* Verify we can reuse the stack */
    int val = 88;
    stk->push(stk, &val);
    print_test_result(stk->size(stk) == 1, "Linked stack usable after clear");
    int *top = (int *)stk->top(stk);
    print_test_result(top != NULL && *top == 88, "Linked stack top after clear returns correct value");

    stk->free(stk);
}

static void test_stack_clear_empty(void) {
    stack *stk = create_stack(ARRAY_STACK);
    stk->clear(stk);
    print_test_result(stk->is_empty(stk), "Clear on empty array stack is safe");
    stk->free(stk);

    stk = create_stack(LINKED_STACK);
    stk->clear(stk);
    print_test_result(stk->is_empty(stk), "Clear on empty linked stack is safe");
    stk->free(stk);
}

/* ── heap clear tests ──────────────────────────────────────────────── */

static void test_binary_heap_clear(void) {
    heap *h = create_heap(BINARY_HEAP, 10, int_compare);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < 5; i++) {
        h->put(h, &values[i]);
    }
    print_test_result(h->size(h) == 5, "Binary heap has 5 elements before clear");

    h->clear(h);
    print_test_result(h->size(h) == 0, "Binary heap is empty after clear");

    /* Verify we can reuse the heap */
    int val = 42;
    h->put(h, &val);
    print_test_result(h->size(h) == 1, "Binary heap usable after clear");
    int *top = (int *)h->peek(h);
    print_test_result(top != NULL && *top == 42, "Binary heap peek after clear returns correct value");

    h->free(h);
}

static int int_compare_fibonacci(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ib - ia);
}

static void test_fibonacci_heap_clear(void) {
    heap *h = create_heap(FIBONACCI_HEAP, 10, int_compare_fibonacci);
    int values[] = {5, 3, 10, 1, 4};
    for (int i = 0; i < 5; i++) {
        h->put(h, &values[i]);
    }
    print_test_result(h->size(h) == 5, "Fibonacci heap has 5 elements before clear");

    h->clear(h);
    print_test_result(h->size(h) == 0, "Fibonacci heap is empty after clear");

    /* Verify we can reuse the heap */
    int val = 42;
    h->put(h, &val);
    print_test_result(h->size(h) == 1, "Fibonacci heap usable after clear");
    int *top = (int *)h->peek(h);
    print_test_result(top != NULL && *top == 42, "Fibonacci heap peek after clear returns correct value");

    h->free(h);
}

static void test_heap_clear_empty(void) {
    heap *h = create_heap(BINARY_HEAP, 4, int_compare);
    h->clear(h);
    print_test_result(h->size(h) == 0, "Clear on empty binary heap is safe");
    h->free(h);

    h = create_heap(FIBONACCI_HEAP, 4, int_compare_fibonacci);
    h->clear(h);
    print_test_result(h->size(h) == 0, "Clear on empty fibonacci heap is safe");
    h->free(h);
}

/* ── map clear tests ───────────────────────────────────────────────── */

static void test_map_clear(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    char k1[] = "key1", k2[] = "key2", k3[] = "key3";
    char v1[] = "val1", v2[] = "val2", v3[] = "val3";
    m->put(m, k1, v1);
    m->put(m, k2, v2);
    m->put(m, k3, v3);
    print_test_result(m->size(m) == 3, "Map has 3 elements before clear");

    m->clear(m);
    print_test_result(m->size(m) == 0, "Map is empty after clear");
    print_test_result(m->get(m, k1) == NULL, "Map get returns NULL after clear");
    print_test_result(m->get(m, k2) == NULL, "Map get returns NULL for second key after clear");

    /* Verify we can reuse the map */
    char k4[] = "newkey";
    char v4[] = "newval";
    m->put(m, k4, v4);
    print_test_result(m->size(m) == 1, "Map usable after clear");
    char *retrieved = (char *)m->get(m, k4);
    print_test_result(retrieved != NULL && strcmp(retrieved, v4) == 0, "Map get after clear returns correct value");

    m->free(m);
}

static void test_map_clear_preserves_capacity(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    int original_capacity = m->capacity(m);

    char k1[] = "key1";
    char v1[] = "val1";
    m->put(m, k1, v1);

    m->clear(m);
    print_test_result(m->capacity(m) == original_capacity, "Map capacity preserved after clear");

    m->free(m);
}

static void test_map_clear_empty(void) {
    map *m = create_map(HASH_MAP, 10, NULL, string_compare);
    m->clear(m);
    print_test_result(m->size(m) == 0, "Clear on empty map is safe");
    m->free(m);
}

static void test_thread_safe_map_clear(void) {
    map *m = create_map(HASH_TABLE, 10, NULL, string_compare);
    char k1[] = "key1";
    char v1[] = "val1";
    m->put(m, k1, v1);
    print_test_result(m->size(m) == 1, "Thread-safe map has 1 element before clear");

    m->clear(m);
    print_test_result(m->size(m) == 0, "Thread-safe map is empty after clear");

    /* Verify we can reuse the map */
    m->put(m, k1, v1);
    print_test_result(m->size(m) == 1, "Thread-safe map usable after clear");

    m->free(m);
}

/* ── main ──────────────────────────────────────────────────────────── */

static void run_all_tests(void) {
    /* list */
    test_array_list_clear();
    test_linked_list_clear();
    test_sorted_list_clear();
    test_list_clear_empty();
    /* queue */
    test_queue_clear();
    test_priority_queue_clear();
    test_queue_clear_empty();
    /* stack */
    test_array_stack_clear();
    test_linked_stack_clear();
    test_stack_clear_empty();
    /* heap */
    test_binary_heap_clear();
    test_fibonacci_heap_clear();
    test_heap_clear_empty();
    /* map */
    test_map_clear();
    test_map_clear_preserves_capacity();
    test_map_clear_empty();
    test_thread_safe_map_clear();
}

int main(void) {
    run_all_tests();
    return 0;
}
