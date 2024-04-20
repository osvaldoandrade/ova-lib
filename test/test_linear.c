#include "../include/linear.h"
#include "base_test.h"
#include <tgmath.h>

#define FLOAT_TOL 0.0001

// Function to compare two matrices
int compare_matrices(matrix *m1, matrix *m2) {
    if (m1->rows != m2->rows || m1->cols != m2->cols) {
        return 0;
    }
    for (int i = 0; i < m1->rows; i++) {
        for (int j = 0; j < m1->cols; j++) {
            if (fabs(m1->data[i][j] - m2->data[i][j]) > FLOAT_TOL) {
                return 0;
            }
        }
    }
    return 1;
}

void test_matrix_add() {
    matrix *m1 = create_matrix(2, 2);
    matrix *m2 = create_matrix(2, 2);
    m1->data[0][0] = 1; m1->data[0][1] = 2;
    m1->data[1][0] = 3; m1->data[1][1] = 4;

    m2->data[0][0] = 1; m2->data[0][1] = 1;
    m2->data[1][0] = 1; m2->data[1][1] = 1;

    matrix *result = m1->add(m1, m2);
    matrix *expected = create_matrix(2, 2);
    expected->data[0][0] = 2; expected->data[0][1] = 3;
    expected->data[1][0] = 4; expected->data[1][1] = 5;

    print_test_result(compare_matrices(result, expected), "Matrix Add");

    m1->destroy(m1);
    m2->destroy(m2);
    result->destroy(result);
    expected->destroy(expected);
}

void test_matrix_subtract() {
    matrix *m1 = create_matrix(2, 2);
    matrix *m2 = create_matrix(2, 2);
    m1->data[0][0] = 5; m1->data[0][1] = 5;
    m1->data[1][0] = 5; m1->data[1][1] = 5;

    m2->data[0][0] = 1; m2->data[0][1] = 1;
    m2->data[1][0] = 1; m2->data[1][1] = 1;

    matrix *result = m1->subtract(m1, m2);
    matrix *expected = create_matrix(2, 2);
    expected->data[0][0] = 4; expected->data[0][1] = 4;
    expected->data[1][0] = 4; expected->data[1][1] = 4;

    print_test_result(compare_matrices(result, expected), "Matrix Subtract");

    m1->destroy(m1);
    m2->destroy(m2);
    result->destroy(result);
    expected->destroy(expected);
}

void test_matrix_multiply() {
    // Create matrices with appropriate dimensions for multiplication
    matrix *m1 = create_matrix(2, 3);
    matrix *m2 = create_matrix(3, 2);

    // Initialize m1
    m1->data[0][0] = 1; m1->data[0][1] = 2; m1->data[0][2] = 3;
    m1->data[1][0] = 4; m1->data[1][1] = 5; m1->data[1][2] = 6;

    // Initialize m2
    m2->data[0][0] = 7; m2->data[0][1] = 8;
    m2->data[1][0] = 9; m2->data[1][1] = 10;
    m2->data[2][0] = 11; m2->data[2][1] = 12;

    // Expected result of m1 * m2
    matrix *expected = create_matrix(2, 2);
    expected->data[0][0] = 58;  // 1*7 + 2*9 + 3*11
    expected->data[0][1] = 64;  // 1*8 + 2*10 + 3*12
    expected->data[1][0] = 139; // 4*7 + 5*9 + 6*11
    expected->data[1][1] = 154; // 4*8 + 5*10 + 6*12

    // Perform multiplication
    matrix *result = m1->multiply(m1, m2);

    // Check if the result matches the expected matrix
    int passed = compare_matrices(result, expected);

    // Print the test result
    print_test_result(passed, "Matrix Multiply");

    // Free the matrices
    m1->destroy(m1);
    m2->destroy(m2);
    result->destroy(result);
    expected->destroy(expected);
}

void test_matrix_determinant() {
    // Create a matrix with a known determinant
    matrix *m = create_matrix(3, 3);
    m->data[0][0] = 1; m->data[0][1] = 2; m->data[0][2] = 3;
    m->data[1][0] = 0; m->data[1][1] = 1; m->data[1][2] = 4;
    m->data[2][0] = 5; m->data[2][1] = 6; m->data[2][2] = 0;

    // The determinant can be calculated manually:
    // Det = 1*(1*0 - 4*6) - 2*(0*0 - 4*5) + 3*(0*6 - 1*5)
    //     = 1*(-24) - 2*(-20) + 3*(-5)
    //     = -24 + 40 - 15
    //     = 1
    double expected = 1;

    // Variable to store error status
    int error = 0;

    // Calculate the determinant
    double result = m->determinant(m, &error);

    // Check if the result matches the expected determinant
    print_test_result(result == expected && error == 0, "Matrix Determinant");

    // Free the matrix
    m->destroy(m);
}

void test_matrix_transpose() {
    // Create a matrix
    matrix *m = create_matrix(2, 3);
    m->data[0][0] = 1; m->data[0][1] = 2; m->data[0][2] = 3;
    m->data[1][0] = 4; m->data[1][1] = 5; m->data[1][2] = 6;

    // Expected result of the transpose
    matrix *expected = create_matrix(3, 2);
    expected->data[0][0] = 1; expected->data[1][0] = 2; expected->data[2][0] = 3;
    expected->data[0][1] = 4; expected->data[1][1] = 5; expected->data[2][1] = 6;

    // Transpose the matrix
    matrix *result = m->transpose(m);

    // Check if the result matches the expected matrix
    int passed = compare_matrices(result, expected);

    // Print the test result
    print_test_result(passed, "Matrix Transpose");

    // Free the matrices
    m->destroy(m);
    result->destroy(result);
    expected->destroy(expected);
}

void test_matrix_inverse() {
    // Create a 2x2 matrix
    matrix *m = create_matrix(2, 2);
    m->data[0][0] = 4; m->data[0][1] = 7;
    m->data[1][0] = 2; m->data[1][1] = 6;

    // Expected inverse of the matrix
    // Inverse calculated manually: 1/Det * [d, -b; -c, a]
    // Det = 4*6 - 7*2 = 10
    matrix *expected = create_matrix(2, 2);
    expected->data[0][0] = 0.6;  expected->data[0][1] = -0.7;
    expected->data[1][0] = -0.2; expected->data[1][1] = 0.4;

    // Calculate the inverse
    matrix *result = m->inverse(m);

    // Check if the result matches the expected matrix
    int passed = compare_matrices(result, expected);

    // Print the test result
    print_test_result(passed, "Matrix Inverse");

    // Free the matrices
    m->destroy(m);
    result->destroy(result);
    expected->destroy(expected);
}

void run_all_tests() {
    test_matrix_add();
    test_matrix_subtract();
    test_matrix_multiply();
    test_matrix_determinant();
    test_matrix_transpose();
    test_matrix_inverse();
}

int main() {
    run_all_tests();
    return 0;
}
