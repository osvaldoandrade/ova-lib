#include "../include/matrix.h"
#include "base_test.h"
#include <tgmath.h>

#define FLOAT_TOL 0.0001

static void set_matrix_value(matrix *m, int row, int col, double value) {
    assert(m->set(m, row, col, value) == 0);
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

void test_matrix_add(void) {
    matrix *m1 = create_matrix(2, 2);
    matrix *m2 = create_matrix(2, 2);
    set_matrix_value(m1, 0, 0, 1);
    set_matrix_value(m1, 0, 1, 2);
    set_matrix_value(m1, 1, 0, 3);
    set_matrix_value(m1, 1, 1, 4);

    set_matrix_value(m2, 0, 0, 1);
    set_matrix_value(m2, 0, 1, 1);
    set_matrix_value(m2, 1, 0, 1);
    set_matrix_value(m2, 1, 1, 1);

    matrix *result = m1->add(m1, m2);
    matrix *expected = create_matrix(2, 2);
    set_matrix_value(expected, 0, 0, 2);
    set_matrix_value(expected, 0, 1, 3);
    set_matrix_value(expected, 1, 0, 4);
    set_matrix_value(expected, 1, 1, 5);

    print_test_result(compare_matrices(result, expected), "Matrix Add");

    m1->free(m1);
    m2->free(m2);
    result->free(result);
    expected->free(expected);
}

void test_matrix_subtract(void) {
    matrix *m1 = create_matrix(2, 2);
    matrix *m2 = create_matrix(2, 2);
    set_matrix_value(m1, 0, 0, 5);
    set_matrix_value(m1, 0, 1, 5);
    set_matrix_value(m1, 1, 0, 5);
    set_matrix_value(m1, 1, 1, 5);

    set_matrix_value(m2, 0, 0, 1);
    set_matrix_value(m2, 0, 1, 1);
    set_matrix_value(m2, 1, 0, 1);
    set_matrix_value(m2, 1, 1, 1);

    matrix *result = m1->subtract(m1, m2);
    matrix *expected = create_matrix(2, 2);
    set_matrix_value(expected, 0, 0, 4);
    set_matrix_value(expected, 0, 1, 4);
    set_matrix_value(expected, 1, 0, 4);
    set_matrix_value(expected, 1, 1, 4);

    print_test_result(compare_matrices(result, expected), "Matrix Subtract");

    m1->free(m1);
    m2->free(m2);
    result->free(result);
    expected->free(expected);
}

void test_matrix_multiply(void) {
    matrix *m1 = create_matrix(2, 3);
    matrix *m2 = create_matrix(3, 2);

    set_matrix_value(m1, 0, 0, 1);
    set_matrix_value(m1, 0, 1, 2);
    set_matrix_value(m1, 0, 2, 3);
    set_matrix_value(m1, 1, 0, 4);
    set_matrix_value(m1, 1, 1, 5);
    set_matrix_value(m1, 1, 2, 6);

    set_matrix_value(m2, 0, 0, 7);
    set_matrix_value(m2, 0, 1, 8);
    set_matrix_value(m2, 1, 0, 9);
    set_matrix_value(m2, 1, 1, 10);
    set_matrix_value(m2, 2, 0, 11);
    set_matrix_value(m2, 2, 1, 12);

    matrix *expected = create_matrix(2, 2);
    set_matrix_value(expected, 0, 0, 58);
    set_matrix_value(expected, 0, 1, 64);
    set_matrix_value(expected, 1, 0, 139);
    set_matrix_value(expected, 1, 1, 154);

    matrix *result = m1->multiply(m1, m2);

    print_test_result(compare_matrices(result, expected), "Matrix Multiply");

    m1->free(m1);
    m2->free(m2);
    result->free(result);
    expected->free(expected);
}

void test_matrix_determinant(void) {
    matrix *m = create_matrix(3, 3);
    set_matrix_value(m, 0, 0, 1);
    set_matrix_value(m, 0, 1, 2);
    set_matrix_value(m, 0, 2, 3);
    set_matrix_value(m, 1, 0, 0);
    set_matrix_value(m, 1, 1, 1);
    set_matrix_value(m, 1, 2, 4);
    set_matrix_value(m, 2, 0, 5);
    set_matrix_value(m, 2, 1, 6);
    set_matrix_value(m, 2, 2, 0);

    double expected = 1;
    int error = 0;
    double result = m->determinant(m, &error);

    print_test_result(result == expected && error == 0, "Matrix Determinant");

    m->free(m);
}

void test_matrix_transpose(void) {
    matrix *m = create_matrix(2, 3);
    set_matrix_value(m, 0, 0, 1);
    set_matrix_value(m, 0, 1, 2);
    set_matrix_value(m, 0, 2, 3);
    set_matrix_value(m, 1, 0, 4);
    set_matrix_value(m, 1, 1, 5);
    set_matrix_value(m, 1, 2, 6);

    matrix *expected = create_matrix(3, 2);
    set_matrix_value(expected, 0, 0, 1);
    set_matrix_value(expected, 1, 0, 2);
    set_matrix_value(expected, 2, 0, 3);
    set_matrix_value(expected, 0, 1, 4);
    set_matrix_value(expected, 1, 1, 5);
    set_matrix_value(expected, 2, 1, 6);

    matrix *result = m->transpose(m);

    print_test_result(compare_matrices(result, expected), "Matrix Transpose");

    m->free(m);
    result->free(result);
    expected->free(expected);
}

void test_matrix_inverse(void) {
    matrix *m = create_matrix(2, 2);
    set_matrix_value(m, 0, 0, 4);
    set_matrix_value(m, 0, 1, 7);
    set_matrix_value(m, 1, 0, 2);
    set_matrix_value(m, 1, 1, 6);

    matrix *expected = create_matrix(2, 2);
    set_matrix_value(expected, 0, 0, 0.6);
    set_matrix_value(expected, 0, 1, -0.7);
    set_matrix_value(expected, 1, 0, -0.2);
    set_matrix_value(expected, 1, 1, 0.4);

    matrix *result = m->inverse(m);

    print_test_result(compare_matrices(result, expected), "Matrix Inverse");

    m->free(m);
    result->free(result);
    expected->free(expected);
}

void run_all_tests(void) {
    test_matrix_add();
    test_matrix_subtract();
    test_matrix_multiply();
    test_matrix_determinant();
    test_matrix_transpose();
    test_matrix_inverse();
}

int main(void) {
    run_all_tests();
    return 0;
}
