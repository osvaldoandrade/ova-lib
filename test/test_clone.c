#include "base_test.h"
#include "../include/list.h"
#include "../include/queue.h"
#include "../include/stack.h"
#include "../include/deque.h"
#include "../include/map.h"
#include "../include/set.h"
#include <stdlib.h>
#include <string.h>

static void *int_copier(void *elem) {
    int *copy = malloc(sizeof(int));
    if (copy) {
        *copy = *(int *)elem;
    }
    return copy;
}

static int int_compare(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static int int_hash(void *key, int capacity) {
    if (!key || capacity <= 0) {
        return 0;
    }
    return *(int *)key % capacity;
}

/* ── List tests ──────────────────────────────────────────────────────── */

void test_list_clone_shallow_array(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int a = 1, b = 2, c = 3;
    lst->insert(lst, &a, 0);
    lst->insert(lst, &b, 1);
    lst->insert(lst, &c, 2);

    list *clone = lst->clone_shallow(lst);

    int same_size = (clone->size(clone) == lst->size(lst));
    int same_ptrs = (clone->get(clone, 0) == lst->get(lst, 0))
                 && (clone->get(clone, 1) == lst->get(lst, 1))
                 && (clone->get(clone, 2) == lst->get(lst, 2));

    print_test_result(same_size && same_ptrs,
                      "Array list shallow clone has same size and same pointers");

    clone->free(clone);
    lst->free(lst);
}

void test_list_clone_shallow_linked(void) {
    list *lst = create_list(LINKED_LIST, 0, NULL);
    int a = 10, b = 20, c = 30;
    lst->insert(lst, &a, 0);
    lst->insert(lst, &b, 1);
    lst->insert(lst, &c, 2);

    list *clone = lst->clone_shallow(lst);

    int same_size = (clone->size(clone) == lst->size(lst));
    int same_ptrs = (clone->get(clone, 0) == lst->get(lst, 0))
                 && (clone->get(clone, 1) == lst->get(lst, 1))
                 && (clone->get(clone, 2) == lst->get(lst, 2));

    print_test_result(same_size && same_ptrs,
                      "Linked list shallow clone has same size and same pointers");

    clone->free(clone);
    lst->free(lst);
}

void test_list_clone_deep_array(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    int a = 100, b = 200, c = 300;
    lst->insert(lst, &a, 0);
    lst->insert(lst, &b, 1);
    lst->insert(lst, &c, 2);

    list *clone = lst->clone_deep(lst, int_copier);

    int same_size = (clone->size(clone) == lst->size(lst));
    int same_vals = (*(int *)clone->get(clone, 0) == *(int *)lst->get(lst, 0))
                 && (*(int *)clone->get(clone, 1) == *(int *)lst->get(lst, 1))
                 && (*(int *)clone->get(clone, 2) == *(int *)lst->get(lst, 2));
    int diff_ptrs = (clone->get(clone, 0) != lst->get(lst, 0))
                 && (clone->get(clone, 1) != lst->get(lst, 1))
                 && (clone->get(clone, 2) != lst->get(lst, 2));

    print_test_result(same_size && same_vals && diff_ptrs,
                      "Array list deep clone has same values but different pointers");

    for (int i = 0; i < clone->size(clone); i++) {
        free(clone->get(clone, i));
    }
    clone->free(clone);
    lst->free(lst);
}

void test_list_clone_deep_linked(void) {
    list *lst = create_list(LINKED_LIST, 0, NULL);
    int a = 100, b = 200, c = 300;
    lst->insert(lst, &a, 0);
    lst->insert(lst, &b, 1);
    lst->insert(lst, &c, 2);

    list *clone = lst->clone_deep(lst, int_copier);

    int same_size = (clone->size(clone) == lst->size(lst));
    int same_vals = (*(int *)clone->get(clone, 0) == *(int *)lst->get(lst, 0))
                 && (*(int *)clone->get(clone, 1) == *(int *)lst->get(lst, 1))
                 && (*(int *)clone->get(clone, 2) == *(int *)lst->get(lst, 2));
    int diff_ptrs = (clone->get(clone, 0) != lst->get(lst, 0))
                 && (clone->get(clone, 1) != lst->get(lst, 1))
                 && (clone->get(clone, 2) != lst->get(lst, 2));

    print_test_result(same_size && same_vals && diff_ptrs,
                      "Linked list deep clone has same values but different pointers");

    for (int i = 0; i < clone->size(clone); i++) {
        free(clone->get(clone, i));
    }
    clone->free(clone);
    lst->free(lst);
}

void test_list_clone_shallow_empty(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);

    list *clone = lst->clone_shallow(lst);

    print_test_result(clone != NULL && clone->size(clone) == 0,
                      "Shallow clone of empty list has size 0");

    clone->free(clone);
    lst->free(lst);
}

void test_list_clone_null(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);

    list *shallow = lst->clone_shallow(NULL);
    list *deep = lst->clone_deep(NULL, int_copier);

    print_test_result(shallow == NULL && deep == NULL,
                      "clone_shallow(NULL) and clone_deep(NULL) return NULL");

    lst->free(lst);
}

/* ── Queue tests ─────────────────────────────────────────────────────── */

void test_queue_clone_shallow(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int a = 1, b = 2, c = 3;
    q->enqueue(q, &a);
    q->enqueue(q, &b);
    q->enqueue(q, &c);

    queue *clone = q->clone_shallow(q);

    int same_size = (clone->size(clone) == q->size(q));

    void *orig_first = q->dequeue(q);
    void *clone_first = clone->dequeue(clone);

    print_test_result(same_size && orig_first == clone_first,
                      "Queue shallow clone has same size and same element pointers");

    clone->free(clone);
    q->free(q);
}

void test_queue_clone_deep(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int a = 10, b = 20, c = 30;
    q->enqueue(q, &a);
    q->enqueue(q, &b);
    q->enqueue(q, &c);

    queue *clone = q->clone_deep(q, int_copier);

    int same_size = (clone->size(clone) == q->size(q));

    int *orig_first = (int *)q->dequeue(q);
    int *clone_first = (int *)clone->dequeue(clone);

    int same_val = (*orig_first == *clone_first);
    int diff_ptr = (orig_first != clone_first);

    print_test_result(same_size && same_val && diff_ptr,
                      "Queue deep clone has same values but different pointers");

    free(clone_first);
    int *c2 = (int *)clone->dequeue(clone);
    free(c2);
    int *c3 = (int *)clone->dequeue(clone);
    free(c3);
    clone->free(clone);
    q->free(q);
}

/* ── Stack tests ─────────────────────────────────────────────────────── */

void test_stack_clone_shallow(void) {
    stack *stk = create_stack(ARRAY_STACK);
    int a = 5, b = 10, c = 15;
    stk->push(stk, &a);
    stk->push(stk, &b);
    stk->push(stk, &c);

    stack *clone = stk->clone_shallow(stk);

    int same_size = (clone->size(clone) == stk->size(stk));
    int same_top = (clone->top(clone) == stk->top(stk));

    print_test_result(same_size && same_top,
                      "Stack shallow clone has same size and same top pointer");

    clone->free(clone);
    stk->free(stk);
}

void test_stack_clone_deep(void) {
    stack *stk = create_stack(ARRAY_STACK);
    int a = 5, b = 10, c = 15;
    stk->push(stk, &a);
    stk->push(stk, &b);
    stk->push(stk, &c);

    stack *clone = stk->clone_deep(stk, int_copier);

    int same_size = (clone->size(clone) == stk->size(stk));
    int *orig_top = (int *)stk->top(stk);
    int *clone_top = (int *)clone->top(clone);
    int same_val = (*orig_top == *clone_top);
    int diff_ptr = (orig_top != clone_top);

    print_test_result(same_size && same_val && diff_ptr,
                      "Stack deep clone has same top value but different pointer");

    int *p1 = (int *)clone->pop(clone);
    free(p1);
    int *p2 = (int *)clone->pop(clone);
    free(p2);
    int *p3 = (int *)clone->pop(clone);
    free(p3);
    clone->free(clone);
    stk->free(stk);
}

/* ── Deque tests ─────────────────────────────────────────────────────── */

void test_deque_clone_shallow(void) {
    deque *d = create_deque(10);
    int a = 1, b = 2, c = 3;
    d->push_back(d, &a);
    d->push_back(d, &b);
    d->push_back(d, &c);

    deque *clone = d->clone_shallow(d);

    int same_size = (clone->size(clone) == d->size(d));
    int same_ptrs = (clone->get(clone, 0) == d->get(d, 0))
                 && (clone->get(clone, 1) == d->get(d, 1))
                 && (clone->get(clone, 2) == d->get(d, 2));

    print_test_result(same_size && same_ptrs,
                      "Deque shallow clone has same size and same element pointers");

    clone->free(clone);
    d->free(d);
}

void test_deque_clone_deep(void) {
    deque *d = create_deque(10);
    int a = 10, b = 20, c = 30;
    d->push_back(d, &a);
    d->push_back(d, &b);
    d->push_back(d, &c);

    deque *clone = d->clone_deep(d, int_copier);

    int same_size = (clone->size(clone) == d->size(d));
    int same_vals = (*(int *)clone->get(clone, 0) == *(int *)d->get(d, 0))
                 && (*(int *)clone->get(clone, 1) == *(int *)d->get(d, 1))
                 && (*(int *)clone->get(clone, 2) == *(int *)d->get(d, 2));
    int diff_ptrs = (clone->get(clone, 0) != d->get(d, 0))
                 && (clone->get(clone, 1) != d->get(d, 1))
                 && (clone->get(clone, 2) != d->get(d, 2));

    print_test_result(same_size && same_vals && diff_ptrs,
                      "Deque deep clone has same values but different pointers");

    free(clone->get(clone, 0));
    free(clone->get(clone, 1));
    free(clone->get(clone, 2));
    clone->free(clone);
    d->free(d);
}

/* ── Map tests ───────────────────────────────────────────────────────── */

void test_map_clone_shallow(void) {
    map *m = create_map(HASH_MAP, 16, int_hash, int_compare);
    int k1 = 1, v1 = 100;
    int k2 = 2, v2 = 200;
    int k3 = 3, v3 = 300;
    m->put(m, &k1, &v1);
    m->put(m, &k2, &v2);
    m->put(m, &k3, &v3);

    map *clone = m->clone_shallow(m);

    int same_size = (clone->size(clone) == m->size(m));
    int same_vals = (clone->get(clone, &k1) == m->get(m, &k1))
                 && (clone->get(clone, &k2) == m->get(m, &k2))
                 && (clone->get(clone, &k3) == m->get(m, &k3));

    print_test_result(same_size && same_vals,
                      "Map shallow clone has same size and same value pointers");

    clone->free(clone);
    m->free(m);
}

void test_map_clone_deep(void) {
    map *m = create_map(HASH_MAP, 16, int_hash, int_compare);
    int k1 = 1, v1 = 100;
    int k2 = 2, v2 = 200;
    int k3 = 3, v3 = 300;
    m->put(m, &k1, &v1);
    m->put(m, &k2, &v2);
    m->put(m, &k3, &v3);

    map *clone = m->clone_deep(m, int_copier);

    int same_size = (clone->size(clone) == m->size(m));

    int *cv1 = (int *)clone->get(clone, &k1);
    int *cv2 = (int *)clone->get(clone, &k2);
    int *cv3 = (int *)clone->get(clone, &k3);

    int same_vals = (cv1 && *cv1 == v1)
                 && (cv2 && *cv2 == v2)
                 && (cv3 && *cv3 == v3);
    int diff_ptrs = (cv1 != &v1)
                 && (cv2 != &v2)
                 && (cv3 != &v3);

    print_test_result(same_size && same_vals && diff_ptrs,
                      "Map deep clone has same values but different pointers");

    clone->free(clone);
    m->free(m);
}

/* ── Set tests ───────────────────────────────────────────────────────── */

void test_set_clone_shallow(void) {
    set *s = create_set(SET_HASH, int_compare, int_hash);
    int a = 1, b = 2, c = 3;
    s->add(s, &a);
    s->add(s, &b);
    s->add(s, &c);

    set *clone = s->clone_shallow(s);

    int same_size = (clone->size(clone) == s->size(s));
    int contains_all = clone->contains(clone, &a)
                    && clone->contains(clone, &b)
                    && clone->contains(clone, &c);

    print_test_result(same_size && contains_all,
                      "Set shallow clone has same size and contains same elements");

    clone->free(clone);
    s->free(s);
}

void test_set_clone_deep(void) {
    set *s = create_set(SET_HASH, int_compare, int_hash);
    int a = 10, b = 20, c = 30;
    s->add(s, &a);
    s->add(s, &b);
    s->add(s, &c);

    set *clone = s->clone_deep(s, int_copier);

    int same_size = (clone->size(clone) == s->size(s));
    int contains_all = clone->contains(clone, &a)
                    && clone->contains(clone, &b)
                    && clone->contains(clone, &c);

    print_test_result(same_size && contains_all,
                      "Set deep clone has same size and contains same values");

    clone->free(clone);
    s->free(s);
}

/* ── Runner ──────────────────────────────────────────────────────────── */

void run_all_tests(void) {
    test_list_clone_shallow_array();
    test_list_clone_shallow_linked();
    test_list_clone_deep_array();
    test_list_clone_deep_linked();
    test_list_clone_shallow_empty();
    test_list_clone_null();
    test_queue_clone_shallow();
    test_queue_clone_deep();
    test_stack_clone_shallow();
    test_stack_clone_deep();
    test_deque_clone_shallow();
    test_deque_clone_deep();
    test_map_clone_shallow();
    test_map_clone_deep();
    test_set_clone_shallow();
    test_set_clone_deep();
}

int main(void) {
    run_all_tests();
    return 0;
}
