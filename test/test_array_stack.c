#include "base_test.h"
#include "../include/stack.h"
#include "../src/list/array_list.h"

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
    stack *s = create_stack(LINKED_STACK);
    print_test_result(s->is_empty(s), "Stack empty on creation");
    int v = 1;
    s->push(s, &v);
    print_test_result(*(int*)s->top(s) == 1, "Stack top after push");
    s->pop(s);
    print_test_result(s->top(s) == NULL, "Stack top NULL after pop");
    s->free(s);
}

void test_stack_high_volume() {
    stack *s = create_stack(LINKED_STACK);
    const int max_items = 5000;
    int *items = generate_random_int_data(max_items);
    for (int i = 0; i < max_items; i++) {
        s->push(s, &items[i]);
    }
    for (int i = max_items - 1; i >= 0; i--) {
        int *val = (int *)s->pop(s);
        assert_not_null(val);
        assert_int_equal(*val, items[i]);
    }
    print_test_result(s->is_empty(s), "Stack empty after high volume pop");
    free(items);
    s->free(s);
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
