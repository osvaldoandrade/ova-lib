#ifndef MATRIX_INTERNAL_H
#define MATRIX_INTERNAL_H

#include "../../include/matrix.h"

typedef struct matrix_impl {
    double **data;
    int rows;
    int cols;
} matrix_impl;

typedef struct vector_impl {
    double *data;
    int size;
} vector_impl;

static inline matrix_impl *matrix_impl_from_matrix(const matrix *m) {
    return m ? (matrix_impl *)m->impl : NULL;
}

static inline vector_impl *vector_impl_from_vector(const vector *v) {
    return v ? (vector_impl *)v->impl : NULL;
}

#endif // MATRIX_INTERNAL_H
