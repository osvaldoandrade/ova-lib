# ova-lib

ova-lib is a lightweight collection of data structures and algorithms written in ANSI C. It aims to provide simple, self contained containers that can be used across POSIX systems with minimal dependencies.

## Features

- **Lists**: array based, linked and sorted lists
- **Queues**: regular and priority queues backed by heaps
- **Heaps**: binary and Fibonacci implementations
- **Stacks**: array and linked stacks
- **Hash Maps**: simple key/value storage with user supplied hashing
- **Matrices**: basic linear algebra helpers
- **Sorting**: assorted comparison based sorting routines
- **Solver**: utilities such as a simplex implementation

## Building

This project uses CMake (>=3.10). Build the project with:

```bash
git clone https://github.com/osvaldoandrade/ova-lib.git
cd ova-lib
mkdir build && cd build
cmake ..
make
```

This produces both a static and a shared library under `build/lib`. To install them system wide run:

```bash
sudo make install
```

Headers are installed into `/usr/local/include/ova_lib` by default.

## Running the tests

The repository contains a suite of unit tests covering the data structures. After building, execute:

```bash
ctest --output-on-failure
```

All tests should pass and give an overview of how each component is used.

## Example usage

### Queues

Creating and using a simple queue:

```c
#include "queue.h"

int main(void) {
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 10, NULL);
    int value = 42;
    q->enqueue(q, &value);
    int *result = (int*)q->dequeue(q);
    q->free(q);
    return result && *result == 42 ? 0 : 1;
}
```

The queue factory can also create priority queues backed by a heap:

```c
#include "queue.h"

int int_cmp(const void *a, const void *b) {
    return (*(const int*)a) - (*(const int*)b);
}

int main(void) {
    queue *pq = create_queue(QUEUE_TYPE_PRIORITY, 10, int_cmp);
    int v1 = 3, v2 = 7;
    pq->enqueue(pq, &v2);
    pq->enqueue(pq, &v1);
    int *max = (int*)pq->dequeue(pq); /* returns 7 */
    pq->free(pq);
    return max && *max == 7 ? 0 : 1;
}
```

### Sorted lists

Sorted lists preserve ordering on every insertion by relying on a user supplied comparator. Elements are stored as opaque
`void*` pointers, allowing custom data types to be stored as long as the comparator understands how to order them.

```c
#include "list.h"

int int_cmp(const void *a, const void *b) {
    const int lhs = *(const int *)a;
    const int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

int main(void) {
    list *sorted = create_list(SORTED_LIST, 8, int_cmp);
    int values[] = {4, 1, 3};
    for (int i = 0; i < 3; ++i) {
        sorted->insert(sorted, &values[i], i); /* index is ignored */
    }

    int *smallest = (int *)sorted->get(sorted, 0); /* returns pointer to value 1 */
    sorted->free(sorted);
    return smallest && *smallest == 1 ? 0 : 1;
}
```

### Matrices

Working with matrices:

```c
#include "matrix.h"

int main(void) {
    matrix *a = create_matrix(2, 2);
    matrix *b = create_matrix(2, 2);
    a->data[0][0] = 1; a->data[0][1] = 2;
    a->data[1][0] = 3; a->data[1][1] = 4;
    b->data[0][0] = 1; b->data[0][1] = 1;
    b->data[1][0] = 1; b->data[1][1] = 1;
    matrix *sum = a->add(a, b);
    sum->destroy(sum);
    a->destroy(a);
    b->destroy(b);
    return 0;
}
```

### Heaps

Using the heap factory to create a Fibonacci heap:

```c
#include "heap.h"

int int_cmp(const void *a, const void *b) {
    return (*(const int*)a) - (*(const int*)b);
}

int main(void) {
    heap *h = create_heap(FIBONACCI_HEAP, 0, int_cmp);
    int vals[] = {5, 1, 9};
    for (int i = 0; i < 3; ++i)
        h->put(h, &vals[i]);
    int *top = (int*)h->pop(h); /* returns 9 */
    h->free(h);
    return top && *top == 9 ? 0 : 1;
}
```

### Simplex solver

Solving a small linear program:

```c
#include "solver.h"

int main(void) {
    lp_problem *p = create_problem(2, 3);
    double obj[] = {3, 5};
    p->setObjective(p, obj, PROBLEM_MAX);

    double c1[] = {1, 2};
    double c2[] = {-3, 1};
    double c3[] = {1, -1};
    p->addConstraint(p, c1, 14);
    p->addConstraint(p, c2, 0);
    p->addConstraint(p, c3, 2);

    solver *s = create_solver(SOLVER_SIMPLEX);
    matrix *tab;
    if (s->solve(p, &tab) == OPTIMAL) {
        printf("x=%f y=%f z=%f\n", p->solution[0], p->solution[1], p->z_value);
    }
    tab->destroy(tab);
    s->destroy(s);
    /* clean up problem here */
    return 0;
}
```

## License

ova-lib is licensed under the [Apache 2.0](LICENSE) license.
