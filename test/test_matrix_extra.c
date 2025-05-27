#include "../include/matrix.h"
#include "base_test.h"
#include <time.h>

#define FLOAT_TOL 0.0001

int compare_matrices(matrix *m1, matrix *m2) {
    if (m1->rows != m2->rows || m1->cols != m2->cols) return 0;
    for (int i = 0; i < m1->rows; i++) {
        for (int j = 0; j < m1->cols; j++) {
            if (fabs(m1->data[i][j] - m2->data[i][j]) > FLOAT_TOL) return 0;
        }
    }
    return 1;
}

void test_create_matrix_invalid() {
    matrix *m = create_matrix(0, 3);
    print_test_result(m == NULL, "create_matrix should fail on zero dimension");
}

void test_matrix_resize() {
    matrix *m = create_matrix(2, 2);
    m->data[0][0] = 1; m->data[0][1] = 2;
    m->data[1][0] = 3; m->data[1][1] = 4;
    m->resize(m, 3, 3);
    int passed = (m->rows == 3 && m->cols == 3 &&
                  m->data[0][0] == 1 && m->data[1][1] == 4 &&
                  fabs(m->data[2][2]) < FLOAT_TOL);
    print_test_result(passed, "matrix_resize should preserve data and update size");
    m->destroy(m);
}

void test_matrix_copy() {
    matrix *m = create_matrix(2,2);
    m->data[0][0] = 5; m->data[0][1] = 6;
    m->data[1][0] = 7; m->data[1][1] = 8;
    matrix *copy = m->copy(m);
    print_test_result(compare_matrices(m, copy), "matrix_copy should duplicate matrix");
    copy->destroy(copy);
    m->destroy(m);
}

void test_matrix_inverse_singular() {
    matrix *m = create_matrix(2,2);
    m->data[0][0] = 1; m->data[0][1] = 2;
    m->data[1][0] = 2; m->data[1][1] = 4; // determinant zero
    matrix *inv = m->inverse(m);
    print_test_result(inv == NULL, "matrix_inverse should fail for singular matrix");
    if (inv) inv->destroy(inv);
    m->destroy(m);
}

void test_matrix_determinant_non_square() {
    matrix *m = create_matrix(2,3);
    int err = 0;
    double d = m->determinant(m, &err);
    print_test_result(err != 0 && d == 0, "determinant on non square matrix errors");
    m->destroy(m);
}

void test_large_matrix_multiply() {
    const int N = 100;
    matrix *a = create_matrix(N, N);
    matrix *b = create_matrix(N, N);
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            a->data[i][j] = 1;
            b->data[i][j] = 1;
        }
    }
    clock_t start = clock();
    matrix *c = a->multiply(a, b);
    double elapsed_ms = ((double)(clock() - start) / CLOCKS_PER_SEC) * 1000.0;
    int pass = (c != NULL && elapsed_ms < 1500.0);
    print_test_result(pass, "Large matrix multiply within time limit");
    if(c) c->destroy(c);
    a->destroy(a);
    b->destroy(b);
}

void test_vector_resize() {
    vector *v = create_vector(2);
    v->data[0] = 1; v->data[1] = 2;
    v->resize(v, 4);
    int passed = (v->size == 4 && v->data[0] == 1 && v->data[1] == 2 &&
                  fabs(v->data[2]) < FLOAT_TOL && fabs(v->data[3]) < FLOAT_TOL);
    print_test_result(passed, "vector_resize should expand and zero new elements");
    v->destroy(v);
}

void run_all_tests() {
    test_create_matrix_invalid();
    test_matrix_resize();
    test_matrix_copy();
    test_matrix_inverse_singular();
    test_matrix_determinant_non_square();
    test_large_matrix_multiply();
    test_vector_resize();
}

int main() {
    run_all_tests();
    return 0;
}
