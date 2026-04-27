#include "../include/matrix.h"
#include "base_test.h"
#include <math.h>

#define FLOAT_TOL 0.0001

static void set_vector_value(vector *v, int index, double value) {
    int rc = v->set(v, index, value);
    if (rc != 0) {
        fprintf(stderr, "vector set failed at %d\n", index);
        abort();
    }
}

void test_vector_add_simd(void) {
    vector *a = create_vector(5);
    vector *b = create_vector(5);
    for (int i = 0; i < 5; i++) {
        set_vector_value(a, i, (double)(i + 1));
        set_vector_value(b, i, (double)(i + 1) * 10.0);
    }

    vector *result = vector_add_simd(a, b);
    int passed = (result != NULL);
    for (int i = 0; i < 5 && passed; i++) {
        double expected = (double)(i + 1) + (double)(i + 1) * 10.0;
        if (fabs(result->get(result, i) - expected) > FLOAT_TOL) {
            passed = 0;
        }
    }
    print_test_result(passed, "vector_add_simd basic");

    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void test_vector_add_simd_large(void) {
    const int N = 1024;
    vector *a = create_vector(N);
    vector *b = create_vector(N);
    for (int i = 0; i < N; i++) {
        set_vector_value(a, i, (double)i);
        set_vector_value(b, i, (double)(N - i));
    }

    vector *result = vector_add_simd(a, b);
    int passed = (result != NULL && result->size(result) == N);
    for (int i = 0; i < N && passed; i++) {
        if (fabs(result->get(result, i) - (double)N) > FLOAT_TOL) {
            passed = 0;
        }
    }
    print_test_result(passed, "vector_add_simd large vector (1024 elements)");

    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void test_vector_add_simd_size_mismatch(void) {
    vector *a = create_vector(3);
    vector *b = create_vector(5);
    vector *result = vector_add_simd(a, b);
    print_test_result(result == NULL, "vector_add_simd size mismatch returns NULL");
    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void test_vector_add_simd_null(void) {
    vector *a = create_vector(3);
    vector *result = vector_add_simd(a, NULL);
    print_test_result(result == NULL, "vector_add_simd NULL argument returns NULL");
    if (result) result->free(result);
    a->free(a);
}

void test_vector_subtract_simd(void) {
    vector *a = create_vector(5);
    vector *b = create_vector(5);
    for (int i = 0; i < 5; i++) {
        set_vector_value(a, i, (double)(i + 1) * 10.0);
        set_vector_value(b, i, (double)(i + 1));
    }

    vector *result = vector_subtract_simd(a, b);
    int passed = (result != NULL);
    for (int i = 0; i < 5 && passed; i++) {
        double expected = (double)(i + 1) * 10.0 - (double)(i + 1);
        if (fabs(result->get(result, i) - expected) > FLOAT_TOL) {
            passed = 0;
        }
    }
    print_test_result(passed, "vector_subtract_simd basic");

    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void test_vector_subtract_simd_size_mismatch(void) {
    vector *a = create_vector(3);
    vector *b = create_vector(5);
    vector *result = vector_subtract_simd(a, b);
    print_test_result(result == NULL, "vector_subtract_simd size mismatch returns NULL");
    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void test_vector_dot_product_simd(void) {
    vector *a = create_vector(5);
    vector *b = create_vector(5);
    for (int i = 0; i < 5; i++) {
        set_vector_value(a, i, (double)(i + 1));
        set_vector_value(b, i, (double)(i + 1));
    }

    /* dot = 1*1 + 2*2 + 3*3 + 4*4 + 5*5 = 55 */
    double result = vector_dot_product_simd(a, b);
    print_test_result(fabs(result - 55.0) < FLOAT_TOL, "vector_dot_product_simd basic");

    a->free(a);
    b->free(b);
}

void test_vector_dot_product_simd_large(void) {
    const int N = 1024;
    vector *a = create_vector(N);
    vector *b = create_vector(N);
    double expected = 0.0;
    for (int i = 0; i < N; i++) {
        set_vector_value(a, i, 1.0);
        set_vector_value(b, i, (double)(i + 1));
        expected += (double)(i + 1);
    }

    double result = vector_dot_product_simd(a, b);
    print_test_result(fabs(result - expected) < 0.01,
                      "vector_dot_product_simd large vector (1024 elements)");

    a->free(a);
    b->free(b);
}

void test_vector_dot_product_simd_size_mismatch(void) {
    vector *a = create_vector(3);
    vector *b = create_vector(5);
    double result = vector_dot_product_simd(a, b);
    print_test_result(result == 0.0, "vector_dot_product_simd size mismatch returns 0");
    a->free(a);
    b->free(b);
}

void test_vector_scale_simd(void) {
    vector *v = create_vector(5);
    for (int i = 0; i < 5; i++) {
        set_vector_value(v, i, (double)(i + 1));
    }

    vector *result = vector_scale_simd(v, 3.0);
    int passed = (result != NULL);
    for (int i = 0; i < 5 && passed; i++) {
        double expected = (double)(i + 1) * 3.0;
        if (fabs(result->get(result, i) - expected) > FLOAT_TOL) {
            passed = 0;
        }
    }
    print_test_result(passed, "vector_scale_simd basic");

    if (result) result->free(result);
    v->free(v);
}

void test_vector_scale_simd_large(void) {
    const int N = 1024;
    vector *v = create_vector(N);
    for (int i = 0; i < N; i++) {
        set_vector_value(v, i, (double)(i + 1));
    }

    vector *result = vector_scale_simd(v, 0.5);
    int passed = (result != NULL && result->size(result) == N);
    for (int i = 0; i < N && passed; i++) {
        double expected = (double)(i + 1) * 0.5;
        if (fabs(result->get(result, i) - expected) > FLOAT_TOL) {
            passed = 0;
        }
    }
    print_test_result(passed, "vector_scale_simd large vector (1024 elements)");

    if (result) result->free(result);
    v->free(v);
}

void test_vector_scale_simd_null(void) {
    vector *result = vector_scale_simd(NULL, 2.0);
    print_test_result(result == NULL, "vector_scale_simd NULL returns NULL");
    if (result) result->free(result);
}

void test_vector_add_simd_single_element(void) {
    vector *a = create_vector(1);
    vector *b = create_vector(1);
    set_vector_value(a, 0, 3.0);
    set_vector_value(b, 0, 7.0);

    vector *result = vector_add_simd(a, b);
    int passed = (result != NULL && fabs(result->get(result, 0) - 10.0) < FLOAT_TOL);
    print_test_result(passed, "vector_add_simd single element");

    if (result) result->free(result);
    a->free(a);
    b->free(b);
}

void run_all_tests(void) {
    test_vector_add_simd();
    test_vector_add_simd_large();
    test_vector_add_simd_size_mismatch();
    test_vector_add_simd_null();
    test_vector_subtract_simd();
    test_vector_subtract_simd_size_mismatch();
    test_vector_dot_product_simd();
    test_vector_dot_product_simd_large();
    test_vector_dot_product_simd_size_mismatch();
    test_vector_scale_simd();
    test_vector_scale_simd_large();
    test_vector_scale_simd_null();
    test_vector_add_simd_single_element();
}

int main(void) {
    run_all_tests();
    return 0;
}
