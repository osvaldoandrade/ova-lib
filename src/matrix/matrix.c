#include "../../include/matrix.h"
#include "matrix_internal.h"

#include <float.h>
#include <string.h>

#ifdef __AVX__
#include <immintrin.h>
#endif

#define STRASSEN_THRESHOLD 64

static double matrix_get_method(const matrix *self, int row, int col);
static int matrix_set_method(matrix *self, int row, int col, double value);
static int matrix_rows_method(const matrix *self);
static int matrix_cols_method(const matrix *self);
static matrix *matrix_add_method(matrix *self, const matrix *other);
static matrix *matrix_subtract_method(matrix *self, const matrix *other);
static matrix *matrix_multiply_method(matrix *self, const matrix *other);
static double matrix_determinant_method(matrix *self, int *error);
static matrix *matrix_transpose_method(matrix *self);
static matrix *matrix_inverse_method(matrix *self);
static int matrix_resize_method(matrix *self, int new_rows, int new_cols);
static matrix *matrix_copy_method(matrix *self);
static void matrix_print_method(const matrix *self);
static void matrix_free_method(matrix *self);

static double vector_get_method(const vector *self, int index);
static int vector_set_method(vector *self, int index, double value);
static int vector_size_method(const vector *self);
static int vector_resize_method(vector *self, int new_size);
static vector *vector_copy_method(const vector *self);
static void vector_print_method(const vector *self);
static void vector_free_method(vector *self);

static matrix *allocate_matrix_object(void) {
    matrix *out = (matrix *)calloc(1, sizeof(matrix));
    if (!out) {
        return NULL;
    }

    out->get = matrix_get_method;
    out->set = matrix_set_method;
    out->rows = matrix_rows_method;
    out->cols = matrix_cols_method;
    out->add = matrix_add_method;
    out->subtract = matrix_subtract_method;
    out->multiply = matrix_multiply_method;
    out->determinant = matrix_determinant_method;
    out->transpose = matrix_transpose_method;
    out->inverse = matrix_inverse_method;
    out->resize = matrix_resize_method;
    out->copy = matrix_copy_method;
    out->print = matrix_print_method;
    out->free = matrix_free_method;
    return out;
}

static vector *allocate_vector_object(void) {
    vector *out = (vector *)calloc(1, sizeof(vector));
    if (!out) {
        return NULL;
    }

    out->get = vector_get_method;
    out->set = vector_set_method;
    out->size = vector_size_method;
    out->resize = vector_resize_method;
    out->copy = vector_copy_method;
    out->print = vector_print_method;
    out->free = vector_free_method;
    return out;
}

matrix *create_matrix(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return NULL;
    }

    matrix *out = allocate_matrix_object();
    if (!out) {
        return NULL;
    }

    matrix_impl *impl = (matrix_impl *)calloc(1, sizeof(matrix_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->data = (double **)calloc((size_t)rows, sizeof(double *));
    if (!impl->data) {
        free(impl);
        free(out);
        return NULL;
    }

    double *buffer = (double *)calloc((size_t)rows * (size_t)cols, sizeof(double));
    if (!buffer) {
        free(impl->data);
        free(impl);
        free(out);
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        impl->data[i] = buffer + (size_t)i * (size_t)cols;
    }

    impl->rows = rows;
    impl->cols = cols;
    out->impl = impl;
    return out;
}

vector *create_vector(int size) {
    if (size <= 0) {
        return NULL;
    }

    vector *out = allocate_vector_object();
    if (!out) {
        return NULL;
    }

    vector_impl *impl = (vector_impl *)calloc(1, sizeof(vector_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->data = (double *)calloc((size_t)size, sizeof(double));
    if (!impl->data) {
        free(impl);
        free(out);
        return NULL;
    }

    impl->size = size;
    out->impl = impl;
    return out;
}

static double matrix_get_method(const matrix *self, int row, int col) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl || row < 0 || col < 0 || row >= impl->rows || col >= impl->cols) {
        return 0.0;
    }
    return impl->data[row][col];
}

static int matrix_set_method(matrix *self, int row, int col, double value) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl || row < 0 || col < 0 || row >= impl->rows || col >= impl->cols) {
        return -1;
    }
    impl->data[row][col] = value;
    return 0;
}

static int matrix_rows_method(const matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    return impl ? impl->rows : 0;
}

static int matrix_cols_method(const matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    return impl ? impl->cols : 0;
}

static matrix *matrix_add_method(matrix *self, const matrix *other) {
    matrix_impl *lhs = matrix_impl_from_matrix(self);
    matrix_impl *rhs = matrix_impl_from_matrix(other);
    if (!lhs || !rhs) {
        return NULL;
    }
    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
        fprintf(stderr, "Matrix dimension mismatch in add: (%d×%d) vs (%d×%d)\n",
                lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return NULL;
    }

    matrix *result = create_matrix(lhs->rows, lhs->cols);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < lhs->rows; i++) {
        for (int j = 0; j < lhs->cols; j++) {
            out->data[i][j] = lhs->data[i][j] + rhs->data[i][j];
        }
    }
    return result;
}

static matrix *matrix_subtract_method(matrix *self, const matrix *other) {
    matrix_impl *lhs = matrix_impl_from_matrix(self);
    matrix_impl *rhs = matrix_impl_from_matrix(other);
    if (!lhs || !rhs) {
        return NULL;
    }
    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
        fprintf(stderr, "Matrix dimension mismatch in subtract: (%d×%d) vs (%d×%d)\n",
                lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return NULL;
    }

    matrix *result = create_matrix(lhs->rows, lhs->cols);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < lhs->rows; i++) {
        for (int j = 0; j < lhs->cols; j++) {
            out->data[i][j] = lhs->data[i][j] - rhs->data[i][j];
        }
    }
    return result;
}

static matrix *matrix_multiply_method(matrix *self, const matrix *other) {
    matrix_impl *lhs = matrix_impl_from_matrix(self);
    matrix_impl *rhs = matrix_impl_from_matrix(other);
    if (!lhs || !rhs) {
        return NULL;
    }
    if (lhs->cols != rhs->rows) {
        fprintf(stderr, "Matrix dimension mismatch in multiply: (%d×%d) * (%d×%d)\n",
                lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return NULL;
    }

    matrix *result = create_matrix(lhs->rows, rhs->cols);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < lhs->rows; i++) {
        for (int k = 0; k < lhs->cols; k++) {
            double a_ik = lhs->data[i][k];
            for (int j = 0; j < rhs->cols; j++) {
                out->data[i][j] += a_ik * rhs->data[k][j];
            }
        }
    }
    return result;
}

static double matrix_determinant_method(matrix *self, int *error) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl || impl->rows != impl->cols) {
        if (error) {
            *error = 1;
        }
        return 0.0;
    }

    int n = impl->rows;
    double det = 1.0;
    matrix *temp = create_matrix(n, n);
    matrix_impl *tmp = matrix_impl_from_matrix(temp);
    if (!tmp) {
        if (error) {
            *error = 1;
        }
        return 0.0;
    }

    for (int i = 0; i < n; i++) {
        memcpy(tmp->data[i], impl->data[i], (size_t)n * sizeof(double));
    }

    for (int i = 0; i < n; i++) {
        if (tmp->data[i][i] == 0) {
            int row_swapped = 0;
            for (int k = i + 1; k < n; k++) {
                if (tmp->data[k][i] != 0) {
                    for (int j = 0; j < n; j++) {
                        double swap = tmp->data[i][j];
                        tmp->data[i][j] = tmp->data[k][j];
                        tmp->data[k][j] = swap;
                    }
                    det *= -1;
                    row_swapped = 1;
                    break;
                }
            }
            if (!row_swapped) {
                det = 0;
                break;
            }
        }

        for (int k = i + 1; k < n; k++) {
            double factor = tmp->data[k][i] / tmp->data[i][i];
            for (int j = i; j < n; j++) {
                tmp->data[k][j] -= tmp->data[i][j] * factor;
            }
        }

        det *= tmp->data[i][i];
    }

    temp->free(temp);
    if (error) {
        *error = 0;
    }
    return det;
}

static matrix *matrix_transpose_method(matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl) {
        return NULL;
    }

    matrix *result = create_matrix(impl->cols, impl->rows);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < impl->rows; i++) {
        for (int j = 0; j < impl->cols; j++) {
            out->data[j][i] = impl->data[i][j];
        }
    }
    return result;
}

static matrix *matrix_inverse_method(matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl || impl->rows != impl->cols) {
        return NULL;
    }

    int n = impl->rows;
    matrix *inverse = create_matrix(n, n);
    matrix *augmented = create_matrix(n, n * 2);
    matrix_impl *inv_impl = matrix_impl_from_matrix(inverse);
    matrix_impl *aug_impl = matrix_impl_from_matrix(augmented);
    if (!inv_impl || !aug_impl) {
        if (inverse) {
            inverse->free(inverse);
        }
        if (augmented) {
            augmented->free(augmented);
        }
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            aug_impl->data[i][j] = impl->data[i][j];
            aug_impl->data[i][j + n] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (int i = 0; i < n; i++) {
        if (aug_impl->data[i][i] == 0) {
            int found = 0;
            for (int k = i + 1; k < n; k++) {
                if (aug_impl->data[k][i] != 0) {
                    for (int j = 0; j < 2 * n; j++) {
                        double temp = aug_impl->data[i][j];
                        aug_impl->data[i][j] = aug_impl->data[k][j];
                        aug_impl->data[k][j] = temp;
                    }
                    found = 1;
                    break;
                }
            }
            if (!found) {
                inverse->free(inverse);
                augmented->free(augmented);
                return NULL;
            }
        }

        double pivot = aug_impl->data[i][i];
        for (int j = 0; j < 2 * n; j++) {
            aug_impl->data[i][j] /= pivot;
        }

        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = aug_impl->data[k][i];
                for (int j = 0; j < 2 * n; j++) {
                    aug_impl->data[k][j] -= factor * aug_impl->data[i][j];
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inv_impl->data[i][j] = aug_impl->data[i][j + n];
        }
    }

    augmented->free(augmented);
    return inverse;
}

static int matrix_resize_method(matrix *self, int new_rows, int new_cols) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl || new_rows <= 0 || new_cols <= 0) {
        return -1;
    }

    if (impl->rows == new_rows && impl->cols == new_cols) {
        return 0;
    }

    double **old_data = impl->data;
    int old_rows = impl->rows;
    int old_cols = impl->cols;
    int min_rows = old_rows < new_rows ? old_rows : new_rows;
    int copy_cols = old_cols < new_cols ? old_cols : new_cols;

    double *new_buffer = (double *)calloc((size_t)new_rows * (size_t)new_cols, sizeof(double));
    if (!new_buffer) {
        return -1;
    }

    double **new_data = (double **)malloc((size_t)new_rows * sizeof(double *));
    if (!new_data) {
        free(new_buffer);
        return -1;
    }

    for (int i = 0; i < new_rows; i++) {
        new_data[i] = new_buffer + (size_t)i * (size_t)new_cols;
    }

    for (int i = 0; i < min_rows; i++) {
        if (copy_cols > 0) {
            memcpy(new_data[i], old_data[i], (size_t)copy_cols * sizeof(double));
        }
    }

    free(old_data[0]);
    free(old_data);

    impl->data = new_data;
    impl->rows = new_rows;
    impl->cols = new_cols;
    return 0;
}

static matrix *matrix_copy_method(matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl) {
        return NULL;
    }

    matrix *copy = create_matrix(impl->rows, impl->cols);
    matrix_impl *out = matrix_impl_from_matrix(copy);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < impl->rows; i++) {
        memcpy(out->data[i], impl->data[i], (size_t)impl->cols * sizeof(double));
    }

    return copy;
}

static void matrix_free_method(matrix *self) {
    if (!self) {
        return;
    }

    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (impl) {
        if (impl->data) {
            free(impl->data[0]);
            free(impl->data);
        }
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

static void matrix_print_method(const matrix *self) {
    matrix_impl *impl = matrix_impl_from_matrix(self);
    if (!impl) {
        printf("Matrix is NULL\n");
        return;
    }

    for (int i = 0; i < impl->rows; i++) {
        for (int j = 0; j < impl->cols; j++) {
            printf("%9.3f ", impl->data[i][j]);
        }
        printf("\n");
    }
}

/* ---- Strassen helpers (operate on raw double** blocks) ---- */

static int strassen_next_power_of_two(int n) {
    int p = 1;
    while (p < n) {
        p *= 2;
    }
    return p;
}

static double **strassen_alloc_block(int n) {
    double **b = (double **)calloc((size_t)n, sizeof(double *));
    if (!b) {
        return NULL;
    }
    double *buf = (double *)calloc((size_t)n * (size_t)n, sizeof(double));
    if (!buf) {
        free(b);
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        b[i] = buf + (size_t)i * (size_t)n;
    }
    return b;
}

static void strassen_free_block(double **b, int n) {
    (void)n;
    if (!b) {
        return;
    }
    free(b[0]);
    free(b);
}

static void strassen_add_block(double **a, double **b, double **result, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i][j] = a[i][j] + b[i][j];
        }
    }
}

static void strassen_sub_block(double **a, double **b, double **result, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i][j] = a[i][j] - b[i][j];
        }
    }
}

static void strassen_naive_block(double **a, double **b, double **result, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i][j] = 0.0;
        }
    }
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double a_ik = a[i][k];
            for (int j = 0; j < n; j++) {
                result[i][j] += a_ik * b[k][j];
            }
        }
    }
}

static int strassen_recurse(double **a, double **b, double **c, int n) {
    if (n <= STRASSEN_THRESHOLD) {
        strassen_naive_block(a, b, c, n);
        return 0;
    }

    int half = n / 2;

    /* Allocate sub-blocks for A and B quadrants */
    double **a11 = strassen_alloc_block(half);
    double **a12 = strassen_alloc_block(half);
    double **a21 = strassen_alloc_block(half);
    double **a22 = strassen_alloc_block(half);
    double **b11 = strassen_alloc_block(half);
    double **b12 = strassen_alloc_block(half);
    double **b21 = strassen_alloc_block(half);
    double **b22 = strassen_alloc_block(half);

    /* M1..M7 products and two temporaries for sums */
    double **m1 = strassen_alloc_block(half);
    double **m2 = strassen_alloc_block(half);
    double **m3 = strassen_alloc_block(half);
    double **m4 = strassen_alloc_block(half);
    double **m5 = strassen_alloc_block(half);
    double **m6 = strassen_alloc_block(half);
    double **m7 = strassen_alloc_block(half);
    double **t1 = strassen_alloc_block(half);
    double **t2 = strassen_alloc_block(half);

    if (!a11 || !a12 || !a21 || !a22 ||
        !b11 || !b12 || !b21 || !b22 ||
        !m1 || !m2 || !m3 || !m4 || !m5 || !m6 || !m7 ||
        !t1 || !t2) {
        strassen_free_block(a11, half); strassen_free_block(a12, half);
        strassen_free_block(a21, half); strassen_free_block(a22, half);
        strassen_free_block(b11, half); strassen_free_block(b12, half);
        strassen_free_block(b21, half); strassen_free_block(b22, half);
        strassen_free_block(m1, half); strassen_free_block(m2, half);
        strassen_free_block(m3, half); strassen_free_block(m4, half);
        strassen_free_block(m5, half); strassen_free_block(m6, half);
        strassen_free_block(m7, half);
        strassen_free_block(t1, half); strassen_free_block(t2, half);
        return -1;
    }

    /* Split A and B into quadrants */
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            a11[i][j] = a[i][j];
            a12[i][j] = a[i][j + half];
            a21[i][j] = a[i + half][j];
            a22[i][j] = a[i + half][j + half];
            b11[i][j] = b[i][j];
            b12[i][j] = b[i][j + half];
            b21[i][j] = b[i + half][j];
            b22[i][j] = b[i + half][j + half];
        }
    }

    /* M1 = (A11 + A22) * (B11 + B22) */
    strassen_add_block(a11, a22, t1, half);
    strassen_add_block(b11, b22, t2, half);
    if (strassen_recurse(t1, t2, m1, half) != 0) { goto fail; }

    /* M2 = (A21 + A22) * B11 */
    strassen_add_block(a21, a22, t1, half);
    if (strassen_recurse(t1, b11, m2, half) != 0) { goto fail; }

    /* M3 = A11 * (B12 - B22) */
    strassen_sub_block(b12, b22, t1, half);
    if (strassen_recurse(a11, t1, m3, half) != 0) { goto fail; }

    /* M4 = A22 * (B21 - B11) */
    strassen_sub_block(b21, b11, t1, half);
    if (strassen_recurse(a22, t1, m4, half) != 0) { goto fail; }

    /* M5 = (A11 + A12) * B22 */
    strassen_add_block(a11, a12, t1, half);
    if (strassen_recurse(t1, b22, m5, half) != 0) { goto fail; }

    /* M6 = (A21 - A11) * (B11 + B12) */
    strassen_sub_block(a21, a11, t1, half);
    strassen_add_block(b11, b12, t2, half);
    if (strassen_recurse(t1, t2, m6, half) != 0) { goto fail; }

    /* M7 = (A12 - A22) * (B21 + B22) */
    strassen_sub_block(a12, a22, t1, half);
    strassen_add_block(b21, b22, t2, half);
    if (strassen_recurse(t1, t2, m7, half) != 0) { goto fail; }

    /* Combine into C quadrants */
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < half; j++) {
            /* C11 = M1 + M4 - M5 + M7 */
            c[i][j] = m1[i][j] + m4[i][j] - m5[i][j] + m7[i][j];
            /* C12 = M3 + M5 */
            c[i][j + half] = m3[i][j] + m5[i][j];
            /* C21 = M2 + M4 */
            c[i + half][j] = m2[i][j] + m4[i][j];
            /* C22 = M1 - M2 + M3 + M6 */
            c[i + half][j + half] = m1[i][j] - m2[i][j] + m3[i][j] + m6[i][j];
        }
    }

    strassen_free_block(a11, half); strassen_free_block(a12, half);
    strassen_free_block(a21, half); strassen_free_block(a22, half);
    strassen_free_block(b11, half); strassen_free_block(b12, half);
    strassen_free_block(b21, half); strassen_free_block(b22, half);
    strassen_free_block(m1, half); strassen_free_block(m2, half);
    strassen_free_block(m3, half); strassen_free_block(m4, half);
    strassen_free_block(m5, half); strassen_free_block(m6, half);
    strassen_free_block(m7, half);
    strassen_free_block(t1, half); strassen_free_block(t2, half);
    return 0;

fail:
    strassen_free_block(a11, half); strassen_free_block(a12, half);
    strassen_free_block(a21, half); strassen_free_block(a22, half);
    strassen_free_block(b11, half); strassen_free_block(b12, half);
    strassen_free_block(b21, half); strassen_free_block(b22, half);
    strassen_free_block(m1, half); strassen_free_block(m2, half);
    strassen_free_block(m3, half); strassen_free_block(m4, half);
    strassen_free_block(m5, half); strassen_free_block(m6, half);
    strassen_free_block(m7, half);
    strassen_free_block(t1, half); strassen_free_block(t2, half);
    return -1;
}

matrix *matrix_multiply_strassen(matrix *a, matrix *b) {
    matrix_impl *lhs = matrix_impl_from_matrix(a);
    matrix_impl *rhs = matrix_impl_from_matrix(b);
    if (!lhs || !rhs) {
        return NULL;
    }
    if (lhs->cols != rhs->rows) {
        fprintf(stderr, "Matrix dimension mismatch in strassen multiply: (%d×%d) * (%d×%d)\n",
                lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return NULL;
    }

    int orig_m = lhs->rows;
    int orig_n = lhs->cols;   /* == rhs->rows */
    int orig_p = rhs->cols;

    /* Determine padded size (next power of two of max dimension) */
    int max_dim = orig_m;
    if (orig_n > max_dim) { max_dim = orig_n; }
    if (orig_p > max_dim) { max_dim = orig_p; }
    int n = strassen_next_power_of_two(max_dim);

    /* Allocate padded blocks (zero-initialized by calloc) */
    double **pa = strassen_alloc_block(n);
    double **pb = strassen_alloc_block(n);
    double **pc = strassen_alloc_block(n);
    if (!pa || !pb || !pc) {
        strassen_free_block(pa, n);
        strassen_free_block(pb, n);
        strassen_free_block(pc, n);
        return NULL;
    }

    /* Copy data into padded blocks */
    for (int i = 0; i < orig_m; i++) {
        memcpy(pa[i], lhs->data[i], (size_t)orig_n * sizeof(double));
    }
    for (int i = 0; i < orig_n; i++) {
        memcpy(pb[i], rhs->data[i], (size_t)orig_p * sizeof(double));
    }

    if (strassen_recurse(pa, pb, pc, n) != 0) {
        strassen_free_block(pa, n);
        strassen_free_block(pb, n);
        strassen_free_block(pc, n);
        return NULL;
    }

    /* Extract result into a matrix object */
    matrix *result = create_matrix(orig_m, orig_p);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        strassen_free_block(pa, n);
        strassen_free_block(pb, n);
        strassen_free_block(pc, n);
        return NULL;
    }

    for (int i = 0; i < orig_m; i++) {
        memcpy(out->data[i], pc[i], (size_t)orig_p * sizeof(double));
    }

    strassen_free_block(pa, n);
    strassen_free_block(pb, n);
    strassen_free_block(pc, n);
    return result;
}

static double vector_get_method(const vector *self, int index) {
    vector_impl *impl = vector_impl_from_vector(self);
    if (!impl || index < 0 || index >= impl->size) {
        return 0.0;
    }
    return impl->data[index];
}

static int vector_set_method(vector *self, int index, double value) {
    vector_impl *impl = vector_impl_from_vector(self);
    if (!impl || index < 0 || index >= impl->size) {
        return -1;
    }
    impl->data[index] = value;
    return 0;
}

static int vector_size_method(const vector *self) {
    vector_impl *impl = vector_impl_from_vector(self);
    return impl ? impl->size : 0;
}

static int vector_resize_method(vector *self, int new_size) {
    vector_impl *impl = vector_impl_from_vector(self);
    if (!impl || new_size <= 0) {
        return -1;
    }

    double *new_data = (double *)realloc(impl->data, (size_t)new_size * sizeof(double));
    if (!new_data) {
        return -1;
    }

    if (new_size > impl->size) {
        for (int i = impl->size; i < new_size; i++) {
            new_data[i] = 0.0;
        }
    }

    impl->data = new_data;
    impl->size = new_size;
    return 0;
}

static vector *vector_copy_method(const vector *self) {
    vector_impl *impl = vector_impl_from_vector(self);
    if (!impl) {
        return NULL;
    }

    vector *copy = create_vector(impl->size);
    vector_impl *out = vector_impl_from_vector(copy);
    if (!out) {
        return NULL;
    }

    memcpy(out->data, impl->data, (size_t)impl->size * sizeof(double));
    return copy;
}

static void vector_print_method(const vector *self) {
    vector_impl *impl = vector_impl_from_vector(self);
    if (!impl) {
        printf("Vector is NULL\n");
        return;
    }

    printf("[");
    for (int i = 0; i < impl->size; i++) {
        printf("%f", impl->data[i]);
        if (i < impl->size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

static void vector_free_method(vector *self) {
    if (!self) {
        return;
    }

    vector_impl *impl = vector_impl_from_vector(self);
    if (impl) {
        free(impl->data);
        impl->data = NULL;
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

/* ------------------------------------------------------------------ */
/*  SIMD-accelerated vector operations (AVX path + scalar fallback)   */
/* ------------------------------------------------------------------ */

vector *vector_add_simd(vector *a, vector *b) {
    vector_impl *va = vector_impl_from_vector(a);
    vector_impl *vb = vector_impl_from_vector(b);
    if (!va || !vb || va->size != vb->size) {
        return NULL;
    }

    int n = va->size;
    vector *result = create_vector(n);
    vector_impl *out = vector_impl_from_vector(result);
    if (!out) {
        return NULL;
    }

    int i = 0;
#ifdef __AVX__
    for (; i + 3 < n; i += 4) {
        __m256d va4 = _mm256_loadu_pd(&va->data[i]);
        __m256d vb4 = _mm256_loadu_pd(&vb->data[i]);
        _mm256_storeu_pd(&out->data[i], _mm256_add_pd(va4, vb4));
    }
#endif
    for (; i < n; i++) {
        out->data[i] = va->data[i] + vb->data[i];
    }

    return result;
}

vector *vector_subtract_simd(vector *a, vector *b) {
    vector_impl *va = vector_impl_from_vector(a);
    vector_impl *vb = vector_impl_from_vector(b);
    if (!va || !vb || va->size != vb->size) {
        return NULL;
    }

    int n = va->size;
    vector *result = create_vector(n);
    vector_impl *out = vector_impl_from_vector(result);
    if (!out) {
        return NULL;
    }

    int i = 0;
#ifdef __AVX__
    for (; i + 3 < n; i += 4) {
        __m256d va4 = _mm256_loadu_pd(&va->data[i]);
        __m256d vb4 = _mm256_loadu_pd(&vb->data[i]);
        _mm256_storeu_pd(&out->data[i], _mm256_sub_pd(va4, vb4));
    }
#endif
    for (; i < n; i++) {
        out->data[i] = va->data[i] - vb->data[i];
    }

    return result;
}

double vector_dot_product_simd(vector *a, vector *b) {
    vector_impl *va = vector_impl_from_vector(a);
    vector_impl *vb = vector_impl_from_vector(b);
    if (!va || !vb || va->size != vb->size) {
        return 0.0;
    }

    int n = va->size;
    double sum = 0.0;
    int i = 0;

#ifdef __AVX__
    __m256d acc = _mm256_setzero_pd();
    for (; i + 3 < n; i += 4) {
        __m256d va4 = _mm256_loadu_pd(&va->data[i]);
        __m256d vb4 = _mm256_loadu_pd(&vb->data[i]);
        acc = _mm256_add_pd(acc, _mm256_mul_pd(va4, vb4));
    }
    /* Horizontal sum of the 4-wide accumulator. */
    double tmp[4];
    _mm256_storeu_pd(tmp, acc);
    sum = tmp[0] + tmp[1] + tmp[2] + tmp[3];
#endif

    for (; i < n; i++) {
        sum += va->data[i] * vb->data[i];
    }

    return sum;
}

vector *vector_scale_simd(vector *v, double scalar) {
    vector_impl *vi = vector_impl_from_vector(v);
    if (!vi) {
        return NULL;
    }

    int n = vi->size;
    vector *result = create_vector(n);
    vector_impl *out = vector_impl_from_vector(result);
    if (!out) {
        return NULL;
    }

    int i = 0;
#ifdef __AVX__
    __m256d vs = _mm256_set1_pd(scalar);
    for (; i + 3 < n; i += 4) {
        __m256d v4 = _mm256_loadu_pd(&vi->data[i]);
        _mm256_storeu_pd(&out->data[i], _mm256_mul_pd(v4, vs));
    }
#endif
    for (; i < n; i++) {
        out->data[i] = vi->data[i] * scalar;
    }

    return result;
}
