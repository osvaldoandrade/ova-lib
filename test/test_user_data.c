#include "base_test.h"
#include "../include/list.h"
#include "../include/map.h"
#include "../include/set.h"
#include "../include/tree.h"
#include "../include/queue.h"
#include "../include/stack.h"
#include "../include/heap.h"
#include "../include/deque.h"
#include "../include/graph.h"
#include "../include/bloom_filter.h"
#include "../include/matrix.h"
#include "../include/trie.h"
#include "../include/sort.h"
#include "../include/solver.h"
#include "../include/memory_pool.h"

static int dummy_cmp(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

static int dummy_hash(void *key, int capacity) {
    return (*(int *)key) % capacity;
}

void test_list_user_data(void) {
    list *lst = create_list(ARRAY_LIST, 10, NULL);
    assert_not_null(lst);
    assert_true(lst->user_data == NULL);

    int context = 42;
    lst->user_data = &context;
    assert_true(lst->user_data == &context);
    assert_int_equal(42, *(int *)lst->user_data);

    lst->free(lst);
    print_test_result(1, "List user_data");
}

void test_map_user_data(void) {
    map *m = create_map(HASH_MAP, 10, dummy_hash, dummy_cmp);
    assert_not_null(m);
    assert_true(m->user_data == NULL);

    int context = 99;
    m->user_data = &context;
    assert_true(m->user_data == &context);

    m->free(m);
    print_test_result(1, "Map user_data");
}

void test_set_user_data(void) {
    set *s = create_set(SET_HASH, dummy_cmp, dummy_hash);
    assert_not_null(s);
    assert_true(s->user_data == NULL);

    int context = 77;
    s->user_data = &context;
    assert_true(s->user_data == &context);

    s->free(s);
    print_test_result(1, "Set user_data");
}

void test_tree_user_data(void) {
    tree *t = create_tree(TREE_AVL, dummy_cmp);
    assert_not_null(t);
    assert_true(t->user_data == NULL);

    int context = 55;
    t->user_data = &context;
    assert_true(t->user_data == &context);

    t->free(t);
    print_test_result(1, "Tree user_data");
}

void test_queue_user_data(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    assert_not_null(q);
    assert_true(q->user_data == NULL);

    int context = 33;
    q->user_data = &context;
    assert_true(q->user_data == &context);

    q->free(q);
    print_test_result(1, "Queue user_data");
}

void test_stack_user_data(void) {
    stack *s = create_stack(ARRAY_STACK);
    assert_not_null(s);
    assert_true(s->user_data == NULL);

    int context = 11;
    s->user_data = &context;
    assert_true(s->user_data == &context);

    s->free(s);
    print_test_result(1, "Stack user_data");
}

void test_heap_user_data(void) {
    heap *h = create_heap(BINARY_HEAP, 10, dummy_cmp);
    assert_not_null(h);
    assert_true(h->user_data == NULL);

    int context = 22;
    h->user_data = &context;
    assert_true(h->user_data == &context);

    h->free(h);
    print_test_result(1, "Heap user_data");
}

void test_deque_user_data(void) {
    deque *d = create_deque(10);
    assert_not_null(d);
    assert_true(d->user_data == NULL);

    int context = 44;
    d->user_data = &context;
    assert_true(d->user_data == &context);

    d->free(d);
    print_test_result(1, "Deque user_data");
}

void test_graph_user_data(void) {
    graph *g = create_graph(GRAPH_DIRECTED, GRAPH_ADJACENCY_LIST,
                            GRAPH_TRAVERSE_BFS, GRAPH_MIN_PATH_DIJKSTRA);
    assert_not_null(g);
    assert_true(g->user_data == NULL);

    int context = 66;
    g->user_data = &context;
    assert_true(g->user_data == &context);

    g->free(g);
    print_test_result(1, "Graph user_data");
}

void test_bloom_filter_user_data(void) {
    bloom_filter *bf = create_bloom_filter(100, 0.01);
    assert_not_null(bf);
    assert_true(bf->user_data == NULL);

    int context = 88;
    bf->user_data = &context;
    assert_true(bf->user_data == &context);

    bf->free(bf);
    print_test_result(1, "Bloom filter user_data");
}

void test_matrix_user_data(void) {
    matrix *m = create_matrix(3, 3);
    assert_not_null(m);
    assert_true(m->user_data == NULL);

    int context = 100;
    m->user_data = &context;
    assert_true(m->user_data == &context);

    m->free(m);
    print_test_result(1, "Matrix user_data");
}

void test_vector_user_data(void) {
    vector *v = create_vector(5);
    assert_not_null(v);
    assert_true(v->user_data == NULL);

    int context = 200;
    v->user_data = &context;
    assert_true(v->user_data == &context);

    v->free(v);
    print_test_result(1, "Vector user_data");
}

void test_trie_user_data(void) {
    trie *t = create_trie();
    assert_not_null(t);
    assert_true(t->user_data == NULL);

    int context = 300;
    t->user_data = &context;
    assert_true(t->user_data == &context);

    t->free(t);
    print_test_result(1, "Trie user_data");
}

void test_sorter_user_data(void) {
    sorter *s = create_sorter(dummy_cmp);
    assert_not_null(s);
    assert_true(s->user_data == NULL);

    int context = 400;
    s->user_data = &context;
    assert_true(s->user_data == &context);

    s->free(s);
    print_test_result(1, "Sorter user_data");
}

void test_solver_user_data(void) {
    solver *s = create_solver(SOLVER_SIMPLEX);
    assert_not_null(s);
    assert_true(s->user_data == NULL);

    int context = 500;
    s->user_data = &context;
    assert_true(s->user_data == &context);

    s->free(s);
    print_test_result(1, "Solver user_data");
}

void test_lp_problem_user_data(void) {
    lp_problem *p = create_problem(3, 2);
    assert_not_null(p);
    assert_true(p->user_data == NULL);

    int context = 600;
    p->user_data = &context;
    assert_true(p->user_data == &context);

    p->free(p);
    print_test_result(1, "LP problem user_data");
}

void test_memory_pool_user_data(void) {
    memory_pool *pool = create_memory_pool(64, 10);
    assert_not_null(pool);
    assert_true(memory_pool_get_user_data(pool) == NULL);

    int context = 700;
    memory_pool_set_user_data(pool, &context);
    assert_true(memory_pool_get_user_data(pool) == &context);
    assert_int_equal(700, *(int *)memory_pool_get_user_data(pool));

    memory_pool_destroy(pool);
    print_test_result(1, "Memory pool user_data");
}

void test_memory_pool_user_data_null_pool(void) {
    assert_true(memory_pool_get_user_data(NULL) == NULL);
    memory_pool_set_user_data(NULL, NULL);
    print_test_result(1, "Memory pool user_data NULL pool");
}

void run_all_tests(void) {
    test_list_user_data();
    test_map_user_data();
    test_set_user_data();
    test_tree_user_data();
    test_queue_user_data();
    test_stack_user_data();
    test_heap_user_data();
    test_deque_user_data();
    test_graph_user_data();
    test_bloom_filter_user_data();
    test_matrix_user_data();
    test_vector_user_data();
    test_trie_user_data();
    test_sorter_user_data();
    test_solver_user_data();
    test_lp_problem_user_data();
    test_memory_pool_user_data();
    test_memory_pool_user_data_null_pool();
}

int main(void) {
    run_all_tests();
    return 0;
}
