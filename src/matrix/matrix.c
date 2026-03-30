#include "../../include/matrix.h"
#include "matrix_internal.h"

#include <float.h>
#include <string.h>

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

    for (int i = 0; i < rows; i++) {
        impl->data[i] = (double *)calloc((size_t)cols, sizeof(double));
        if (!impl->data[i]) {
            for (int j = 0; j < i; j++) {
                free(impl->data[j]);
            }
            free(impl->data);
            free(impl);
            free(out);
            return NULL;
        }
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
    if (!lhs || !rhs || lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
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
    if (!lhs || !rhs || lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
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
    if (!lhs || !rhs || lhs->cols != rhs->rows) {
        return NULL;
    }

    matrix *result = create_matrix(lhs->rows, rhs->cols);
    matrix_impl *out = matrix_impl_from_matrix(result);
    if (!out) {
        return NULL;
    }

    for (int i = 0; i < lhs->rows; i++) {
        for (int j = 0; j < rhs->cols; j++) {
            out->data[i][j] = 0.0;
            for (int k = 0; k < lhs->cols; k++) {
                out->data[i][j] += lhs->data[i][k] * rhs->data[k][j];
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

    double **new_data = (double **)malloc((size_t)new_rows * sizeof(double *));
    if (!new_data) {
        return -1;
    }

    for (int i = 0; i < new_rows; i++) {
        new_data[i] = NULL;
    }

    for (int i = 0; i < min_rows; i++) {
        double *new_row = (double *)malloc((size_t)new_cols * sizeof(double));
        if (!new_row) {
            goto resize_failure;
        }

        if (copy_cols > 0) {
            memcpy(new_row, old_data[i], (size_t)copy_cols * sizeof(double));
        }
        for (int j = copy_cols; j < new_cols; j++) {
            new_row[j] = 0.0;
        }

        new_data[i] = new_row;
    }

    for (int i = min_rows; i < new_rows; i++) {
        new_data[i] = (double *)calloc((size_t)new_cols, sizeof(double));
        if (!new_data[i]) {
            goto resize_failure;
        }
    }

    for (int i = 0; i < old_rows; i++) {
        free(old_data[i]);
    }
    free(old_data);

    impl->data = new_data;
    impl->rows = new_rows;
    impl->cols = new_cols;
    return 0;

resize_failure:
    for (int i = 0; i < new_rows; i++) {
        free(new_data[i]);
    }
    free(new_data);
    return -1;
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
            for (int i = 0; i < impl->rows; i++) {
                free(impl->data[i]);
            }
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
