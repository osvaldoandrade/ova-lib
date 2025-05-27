#include "base_test.h"
#include "../include/queue.h"
#include <string.h>
#include <time.h>

int int_compare_binary(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

int int_compare_fibonacci(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ib - ia); // Para max-heap; inverter para min-heap
}

void test_priority_queue_empty_initially() {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_compare_binary);
    print_test_result(pq->is_empty(pq), "Priority Queue should be empty after initialization");
    pq->free(pq);
}

void test_priority_queue_dequeue_empty() {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 4, int_compare_binary);
    print_test_result(pq->dequeue(pq) == NULL, "Dequeue on empty priority queue returns NULL");
    pq->free(pq);
}

void test_priority_queue_enqueue_dequeue() {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_compare_binary);
    int data1 = 42;
    pq->enqueue(pq, &data1);
    print_test_result(!pq->is_empty(pq), "Priority Queue should not be empty after enqueue");

    int* dequeued_data = (int*)pq->dequeue(pq);
    print_test_result(dequeued_data != NULL && *dequeued_data == data1, "Dequeued data should match the enqueued data");
    pq->free(pq);
}

void test_priority_queue_multiple_elements() {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_compare_binary);
    int data1 = 42, data2 = 56, data3 = 15;
    pq->enqueue(pq, &data3);
    pq->enqueue(pq, &data2);
    pq->enqueue(pq, &data1);

    int *dequeued_data = (int*)pq->dequeue(pq);
    print_test_result(*dequeued_data == data2, "Highest priority element should be dequeued first (data2)");

    dequeued_data = (int*)pq->dequeue(pq);
    print_test_result(*dequeued_data == data1, "Next highest priority element should be dequeued second (data1)");

    dequeued_data = (int*)pq->dequeue(pq);
    print_test_result(*dequeued_data == data3, "Lowest priority element should be dequeued last (data3)");
    print_test_result(pq->is_empty(pq), "Priority Queue should be empty after all elements are dequeued");

    pq->free(pq);
}

void test_priority_queue_high_volume() {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_compare_binary);
    const int max_data = 1000;
    clock_t start = clock();
    int* data_array = generate_random_int_data(max_data);

    for (int i = 0; i < max_data; i++) {
        pq->enqueue(pq, &data_array[i]);
    }

    char message[100];
    sprintf(message, "Priority Queue should have %d elements after enqueues", max_data);
    print_test_result(pq->size(pq) == max_data, message);

    for (int i = 0; i < max_data; i++) {
        int *dequeued_data = (int*)pq->dequeue(pq);
        assert(dequeued_data != NULL); // Ensuring that we actually dequeue data
    }
    sprintf(message, "Priority Queue should be empty after dequeuing %d elements", max_data);
    print_test_result(pq->is_empty(pq), message);

    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1500.0, "Priority queue high volume within time limit");

    free(data_array);
    pq->free(pq);
}

void run_all_priority_queue_tests() {
    test_priority_queue_empty_initially();
    test_priority_queue_dequeue_empty();
    test_priority_queue_enqueue_dequeue();
    test_priority_queue_multiple_elements();
    test_priority_queue_high_volume();
}

int main() {
    run_all_priority_queue_tests();
    return 0;
}
