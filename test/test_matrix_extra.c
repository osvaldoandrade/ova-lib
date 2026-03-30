#include "../include/matrix.h"
#include "base_test.h"
#include <math.h>
#include <time.h>

#define FLOAT_TOL 0.0001

static void set_matrix_value(matrix *m, int row, int col, double value) {
    assert(m->set(m, row, col, value) == 0);
}

static void set_vector_value(vector *v, int index, double value) {
    assert(v->set(v, index, value) == 0);
}

static int compare_matrices(matrix *m1, matrix *m2) {
    if (m1->rows(m1) != m2->rows(m2) || m1->cols(m1) != m2->cols(m2)) {
        return 0;
    }
    for (int i = 0; i < m1->rows(m1); i++) {
        for (int j = 0; j < m1->cols(m1); j++) {
            if (fabs(m1->get(m1, i, j) - m2->get(m2, i, j)) > FLOAT_TOL) {
                return 0;
            }
        }
    }
    return 1;
}

void test_create_matrix_invalid(void) {
    matrix *m = create_matrix(0, 3);
    print_test_result(m == NULL, "create_matrix should fail on zero dimension");
}

void test_matrix_resize(void) {
    matrix *m = create_matrix(2, 2);
    set_matrix_value(m, 0, 0, 1);
    set_matrix_value(m, 0, 1, 2);
    set_matrix_value(m, 1, 0, 3);
    set_matrix_value(m, 1, 1, 4);

    int result = m->resize(m, 3, 3);
    int passed = (result == 0 &&
                  m->rows(m) == 3 && m->cols(m) == 3 &&
                  m->get(m, 0, 0) == 1 && m->get(m, 1, 1) == 4 &&
                  fabs(m->get(m, 2, 2)) < FLOAT_TOL);
    print_test_result(passed, "matrix_resize should preserve data and update size");
    m->free(m);
}

void test_matrix_resize_shrink_and_expand(void) {
    matrix *m = create_matrix(3, 3);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            set_matrix_value(m, i, j, (double)(i * 3 + j + 1));
        }
    }

    int shrink_result = m->resize(m, 2, 2);
    int shrink_ok = (shrink_result == 0 && m->rows(m) == 2 && m->cols(m) == 2 &&
                     fabs(m->get(m, 0, 0) - 1.0) < FLOAT_TOL &&
                     fabs(m->get(m, 1, 1) - 5.0) < FLOAT_TOL);

    int expand_result = shrink_ok ? m->resize(m, 4, 4) : -1;
    int expand_ok = (shrink_ok && expand_result == 0 &&
                     m->rows(m) == 4 && m->cols(m) == 4 &&
                     fabs(m->get(m, 0, 0) - 1.0) < FLOAT_TOL &&
                     fabs(m->get(m, 1, 1) - 5.0) < FLOAT_TOL &&
                     fabs(m->get(m, 3, 3)) < FLOAT_TOL);

    print_test_result(shrink_ok && expand_ok,
                      "matrix_resize should support shrinking and expanding safely");
    m->free(m);
}

void test_matrix_resize_failure_keeps_state(void) {
    matrix *m = create_matrix(2, 2);
    set_matrix_value(m, 0, 0, 7);
    set_matrix_value(m, 0, 1, 8);
    set_matrix_value(m, 1, 0, 9);
    set_matrix_value(m, 1, 1, 10);

    int result = m->resize(m, 0, 3);
    int passed = (result != 0 &&
                  m->rows(m) == 2 && m->cols(m) == 2 &&
                  fabs(m->get(m, 0, 0) - 7.0) < FLOAT_TOL &&
                  fabs(m->get(m, 1, 1) - 10.0) < FLOAT_TOL);
    print_test_result(passed, "matrix_resize failure should not alter matrix state");
    m->free(m);
}

void test_matrix_copy(void) {
    matrix *m = create_matrix(2, 2);
    set_matrix_value(m, 0, 0, 5);
    set_matrix_value(m, 0, 1, 6);
    set_matrix_value(m, 1, 0, 7);
    set_matrix_value(m, 1, 1, 8);
    matrix *copy = m->copy(m);
    print_test_result(compare_matrices(m, copy), "matrix_copy should duplicate matrix");
    copy->free(copy);
    m->free(m);
}

void test_matrix_inverse_singular(void) {
    matrix *m = create_matrix(2, 2);
    set_matrix_value(m, 0, 0, 1);
    set_matrix_value(m, 0, 1, 2);
    set_matrix_value(m, 1, 0, 2);
    set_matrix_value(m, 1, 1, 4);
    matrix *inv = m->inverse(m);
    print_test_result(inv == NULL, "matrix_inverse should fail for singular matrix");
    if (inv) {
        inv->free(inv);
    }
    m->free(m);
}

void test_matrix_determinant_non_square(void) {
    matrix *m = create_matrix(2, 3);
    int err = 0;
    double d = m->determinant(m, &err);
    print_test_result(err != 0 && d == 0, "determinant on non square matrix errors");
    m->free(m);
}

void test_large_matrix_multiply(void) {
    const int N = 100;
    matrix *a = create_matrix(N, N);
    matrix *b = create_matrix(N, N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            set_matrix_value(a, i, j, 1);
            set_matrix_value(b, i, j, 1);
        }
    }
    clock_t start = clock();
    matrix *c = a->multiply(a, b);
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    int pass = (c != NULL && elapsed_ms < 1500.0);
    print_test_result(pass, "Large matrix multiply within time limit");
    if (c) {
        c->free(c);
    }
    a->free(a);
    b->free(b);
}

void test_vector_resize(void) {
    vector *v = create_vector(2);
    set_vector_value(v, 0, 1);
    set_vector_value(v, 1, 2);
    v->resize(v, 4);
    int passed = (v->size(v) == 4 && v->get(v, 0) == 1 && v->get(v, 1) == 2 &&
                  fabs(v->get(v, 2)) < FLOAT_TOL && fabs(v->get(v, 3)) < FLOAT_TOL);
    print_test_result(passed, "vector_resize should expand and zero new elements");
    v->free(v);
}

void run_all_tests(void) {
    test_create_matrix_invalid();
    test_matrix_resize();
    test_matrix_resize_shrink_and_expand();
    test_matrix_resize_failure_keeps_state();
    test_matrix_copy();
    test_matrix_inverse_singular();
    test_matrix_determinant_non_square();
    test_large_matrix_multiply();
    test_vector_resize();
}

int main(void) {
    run_all_tests();
    return 0;
}
