#include "base_test.h"
#include "../include/queue.h"
#include <string.h>
#include <time.h>

void test_queue_empty_initially(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    print_test_result(q->is_empty(q), "Queue should be empty after initialization");
    q->free(q);
}

void test_queue_dequeue_empty(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 0, NULL);
    print_test_result(q->dequeue(q) == NULL, "Dequeue on empty queue returns NULL");
    q->free(q);
}

void test_queue_enqueue_dequeue_single(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int data1 = 42;
    q->enqueue(q, &data1);
    print_test_result(!q->is_empty(q), "Queue should not be empty after enqueue");

    int *dequeued_data = (int*)q->dequeue(q);
    print_test_result(dequeued_data != NULL && *dequeued_data == data1, "Dequeued data should match the enqueued data");
    q->free(q);
}

void test_queue_enqueue_dequeue_multiple(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int data1 = 42, data2 = 56;
    q->enqueue(q, &data1);
    q->enqueue(q, &data2);

    int *dequeued_data = (int*)q->dequeue(q);
    print_test_result(dequeued_data != NULL && *dequeued_data == data1, "First dequeued data should match the first enqueued");
    dequeued_data = (int*)q->dequeue(q);
    print_test_result(dequeued_data != NULL && *dequeued_data == data2, "Second dequeued data should match the second enqueued");
    print_test_result(q->is_empty(q), "Queue should be empty after dequeuing all elements");
    q->free(q);
}

void test_queue_high_volume(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    const int max_data = 1000;
    clock_t start = clock();
    int* data_array = generate_random_int_data((size_t)max_data);

    for (int i = 0; i < max_data; i++) {
        q->enqueue(q, &data_array[i]);
    }

    for (int i = 0; i < max_data; i++) {
        int *dequeued_data = (int*)q->dequeue(q);
        assert(dequeued_data != NULL && *dequeued_data == data_array[i]);  // Ensures each dequeued item matches the enqueued item
        (void)dequeued_data;
    }
    assert(q->is_empty(q));  // Verifies the queue is empty after all dequeues

    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1000.0, "Queue high volume operations within time limit");

    free(data_array);
    q->free(q);
}

void test_queue_with_string_data(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    char* str_data = generate_random_string_data();
    q->enqueue(q, str_data);

    char* dequeued_str = (char*)q->dequeue(q);
    print_test_result(dequeued_str != NULL && strcmp(dequeued_str, str_data) == 0, "Dequeued string data should match the enqueued string data");
    free(str_data);
    q->free(q);
}

void test_queue_enqueue_error_codes(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int data1 = 42;

    print_test_result(q->enqueue(q, &data1) == OVA_SUCCESS,
                      "Queue enqueue returns OVA_SUCCESS");

    q->free(q);
}

void test_queue_node_recycle(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 0, NULL);
    const int rounds = 8;
    const int batch = 256;
    int values[256];
    for (int i = 0; i < batch; i++) values[i] = i;

    int ok = 1;
    for (int r = 0; r < rounds; r++) {
        for (int i = 0; i < batch; i++) {
            if (q->enqueue(q, &values[i]) != OVA_SUCCESS) { ok = 0; break; }
        }
        for (int i = 0; i < batch; i++) {
            int *out = (int *)q->dequeue(q);
            if (!out || *out != i) { ok = 0; break; }
        }
        if (!q->is_empty(q)) { ok = 0; break; }
    }
    print_test_result(ok, "Queue preserves FIFO order across repeated enqueue/dequeue cycles (freelist reuse)");
    q->free(q);
}

void run_all_queue_tests(void) {
    test_queue_empty_initially();
    test_queue_dequeue_empty();
    test_queue_enqueue_dequeue_single();
    test_queue_enqueue_dequeue_multiple();
    test_queue_high_volume();
    test_queue_with_string_data();
    test_queue_enqueue_error_codes();
    test_queue_node_recycle();
}

int main(void) {
    run_all_queue_tests();
    return 0;
}
