#include "base_test.h"
#include "../include/stack.h"
#include "../src/list/array_list.h"
#include <time.h>

void test_linked_stack_push_pop() {
    stack *stk = create_stack(LINKED_STACK);
    int items_to_test[] = {10, 20, 30, 40, 50};
    int num_items = sizeof(items_to_test) / sizeof(items_to_test[0]);

    for (int i = 0; i < num_items; i++) {
        stk->push(stk, &items_to_test[i]);
    }

    int passed = 1;
    for (int i = num_items - 1; i >= 0; i--) {
        int *retrieved_item = (int *)stk->pop(stk);
        if (*retrieved_item != items_to_test[i]) {
            passed = 0;
            break;
        }
    }
    print_test_result(passed, "Linked Stack push and pop operations");

    stk->free(stk);
}

void test_linked_stack_empty_after_pop() {
    stack *stk = create_stack(LINKED_STACK);
    int item = 123;
    stk->push(stk, &item);
    stk->pop(stk); // Pop the item to make stack empty

    print_test_result(stk->pop(stk) == NULL, "Linked Stack should be empty after pop");

    stk->free(stk);
}

void test_stack_top_behavior() {
    stack *stk = create_stack(LINKED_STACK);
    print_test_result(stk->top(stk) == NULL, "Top on new stack returns NULL");
    int v = 5;
    stk->push(stk, &v);
    print_test_result(*(int*)stk->top(stk) == 5, "Top after push returns element");
    stk->pop(stk);
    print_test_result(stk->top(stk) == NULL, "Top after pop returns NULL");
    stk->free(stk);
}

void test_stack_high_volume() {
    stack *stk = create_stack(LINKED_STACK);
    const int MAX = 1000;
    clock_t start = clock();
    for (int i = 0; i < MAX; i++) {
        stk->push(stk, &i);
    }
    for (int i = 0; i < MAX; i++) {
        stk->pop(stk);
    }
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    print_test_result(elapsed_ms < 1000.0 && stk->is_empty(stk), "Stack high volume within time limit");
    stk->free(stk);
}

void run_all_tests() {
    test_linked_stack_push_pop();
    test_linked_stack_empty_after_pop();
    test_stack_top_behavior();
    test_stack_high_volume();
}

int main() {
    run_all_tests();
    return 0;
}
