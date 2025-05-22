/**
 * @file fibonacci_heap.c
 * @brief Fibonacci heap implementation.
 */

#include "fibonacci_heap.h"
#include <stdlib.h>

/**
 * @brief Allocate a new Fibonacci heap node.
 *
 * @param data Pointer to the data to store in the node.
 * @return Newly allocated node or NULL on failure.
 */
static fib_node *fib_node_create(void *data) {
    fib_node *node = malloc(sizeof(fib_node));
    if (!node) return NULL;

    node->data = data;
    node->degree = 0;
    node->parent = NULL;
    node->child = NULL;
    node->left = node;
    node->right = node;
    node->mark = 0;
    return node;
}

/**
 * @brief Link two nodes in the Fibonacci heap.
 *
 * Makes @p child a child of @p parent during consolidation.
 */
static void fib_node_link(fib_node *child, fib_node *parent) {
    child->left->right = child->right;
    child->right->left = child->left;

    child->parent = parent;
    child->left = child->right = child;
    if (parent->child == NULL) {
        parent->child = child;
    } else {
        child->right = parent->child;
        child->left = parent->child->left;
        parent->child->left->right = child;
        parent->child->left = child;
    }
    parent->degree++;
    child->mark = 0;
}

/**
 * @brief Consolidate the root list of the Fibonacci heap.
 *
 * Performs pairwise merging of trees with the same degree after a delete
 * operation to reduce the number of roots.
 */
static void consolidate(fibonacci_heap *h) {
    int D = 45;
    fib_node* A[45] = {NULL};

    fib_node *start = h->min;
    if (!start) return;

    fib_node *w = start;
    do {
        fib_node *next = w->right;

        if (!w || !w->data) {
            w = next;
            continue;
        }

        fib_node *x = w;
        w = next;
        int d = x->degree;

        while (d < D && A[d]) {
            fib_node *y = A[d];
            if (!y || !y->data) {
                A[d] = NULL;
                break;
            }

            if (h->cmp(x->data, y->data) > 0) {
                fib_node *temp = x;
                x = y;
                y = temp;
            }
            fib_node_link(y, x);
            A[d] = NULL;
            d++;
        }

        if (d < D) {
            A[d] = x;
        }
    } while (w != start);

    h->min = NULL;
    for (int i = 0; i < D; i++) {
        if (A[i]) {
            A[i]->left = A[i]->right = A[i];
            if (!h->min) {
                h->min = A[i];
            } else {
                A[i]->right = h->min->right;
                A[i]->left = h->min;
                h->min->right->left = A[i];
                h->min->right = A[i];
                if (h->cmp(A[i]->data, h->min->data) < 0) {
                    h->min = A[i];
                }
            }
        }
    }
}

/**
 * @brief Insert an element into the Fibonacci heap.
 *
 * @param self Heap wrapper used for insertion.
 * @param data Data to be stored.
 */
static void fibonacci_heap_put(heap *self, void *data) {
    fibonacci_heap *h = (fibonacci_heap *)self->impl;
    fib_node *node = fib_node_create(data);
    if (node) {
        if (!h->min) {
            h->min = node;
        } else {
            node->right = h->min->right;
            node->right->left = node;
            node->left = h->min;
            h->min->right = node;
            if (h->cmp(data, h->min->data) < 0) {
                h->min = node;
            }
        }
        h->n++;
    }
}

/**
 * @brief Remove and return the minimum element from the Fibonacci heap.
 *
 * @param self Heap wrapper whose minimum element will be extracted.
 * @return Pointer to the removed element or NULL if the heap is empty.
 */
static void *fibonacci_heap_pop(heap *self) {
    fibonacci_heap *h = (fibonacci_heap *)self->impl;
    fib_node *z = h->min;
    if (z) {
        if (z->child) {
            fib_node *x = z->child;
            do {
                fib_node *next = x->right;
                x->left->right = x->right;
                x->right->left = x->left;
                x->left = h->min;
                x->right = h->min->right;
                h->min->right = x;
                x->right->left = x;
                x->parent = NULL;
                x = next;
            } while (x != z->child);
        }
        z->left->right = z->right;
        z->right->left = z->left;

        if (z == z->right) {
            h->min = NULL;
        } else {
            h->min = z->right;
            consolidate(h);
        }
        h->n--;
        void *data = z->data;
        free(z);
        return data;
    }
    return NULL;
}

/**
 * @brief Peek at the minimum element without removing it.
 *
 * @param self Heap wrapper to query.
 * @return Pointer to the smallest element or NULL if empty.
 */
static void *fibonacci_heap_peek(const heap *self) {
    fibonacci_heap *h = (fibonacci_heap *)self->impl;
    return h->min ? h->min->data : NULL;
}

/**
 * @brief Return the number of elements stored in the Fibonacci heap.
 *
 * @param self Heap wrapper to query.
 * @return Total number of elements.
 */
static int fibonacci_heap_size(const heap *self) {
    fibonacci_heap *h = (fibonacci_heap *)self->impl;
    return h->n;
}

/**
 * @brief Recursively free a Fibonacci heap node and its children.
 */
static void free_fib_node(fib_node *node) {
    if (node == NULL) return;

    fib_node *cur = node;
    do {
        fib_node *temp = cur;
        cur = cur->right;  // Avança para o próximo nó no nível
        if (temp->child != NULL) {
            free_fib_node(temp->child);  // Recursivamente libera os filhos
        }
        free(temp);  // Libera o nó atual
    } while (cur != node);  // A condição de parada ocorre quando circula de volta ao nó inicial
}

/**
 * @brief Free the Fibonacci heap and all of its nodes.
 *
 * @param self Heap wrapper to destroy.
 */
static void fibonacci_heap_free(heap *self) {
    fibonacci_heap *h = (fibonacci_heap *)self->impl;
    if (h->min != NULL) {
        free_fib_node(h->min);  // Inicia a liberação de memória do nó mínimo
    }
    free(h);
    free(self);
}

/**
 * @brief Create a generic heap backed by a Fibonacci heap implementation.
 *
 * @param compare_function Comparator for ordering elements in the heap.
 * @return Pointer to a heap instance or NULL on failure.
 */
heap *create_fibonacci_heap(comparator compare_function) {
    fibonacci_heap *fh = malloc(sizeof(fibonacci_heap));
    if (!fh) return NULL;

    fh->min = NULL;
    fh->n = 0;
    fh->cmp = compare_function;

    heap *h = malloc(sizeof(heap));
    if (!h) {
        free(fh);
        return NULL;
    }

    h->impl = fh;
    h->put = fibonacci_heap_put;
    h->pop = fibonacci_heap_pop;
    h->peek = fibonacci_heap_peek;
    h->size = fibonacci_heap_size;
    h->free = fibonacci_heap_free;
    return h;
}