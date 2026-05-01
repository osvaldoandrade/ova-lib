#ifndef MATRIX_H
#define MATRIX_H

#include "types.h"

/**
 * @brief Public matrix object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct matrix {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Read a matrix element.
     *
     * @param self Matrix instance.
     * @param row Zero-based row index.
     * @param col Zero-based column index.
     * @return Element value, or 0.0 when out of bounds.
     */
    double (*get)(const struct matrix *self, int row, int col);

    /**
     * @brief Write a matrix element.
     *
     * @param self Matrix instance.
     * @param row Zero-based row index.
     * @param col Zero-based column index.
     * @param value New element value.
     * @return 0 on success, -1 on invalid coordinates.
     */
    int (*set)(struct matrix *self, int row, int col, double value);

    /**
     * @brief Return the row count.
     *
     * @param self Matrix instance.
     * @return Number of rows.
     */
    int (*rows)(const struct matrix *self);

    /**
     * @brief Return the column count.
     *
     * @param self Matrix instance.
     * @return Number of columns.
     */
    int (*cols)(const struct matrix *self);

    /**
     * @brief Add another matrix and return the result.
     *
     * @param self Matrix instance.
     * @param other Right-hand matrix.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*add)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Subtract another matrix and return the result.
     *
     * @param self Matrix instance.
     * @param other Right-hand matrix.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*subtract)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Multiply by another matrix and return the result.
     *
     * @param self Matrix instance.
     * @param other Right-hand matrix.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*multiply)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Compute the matrix determinant.
     *
     * @param self Matrix instance.
     * @param error Output error flag. Set to 0 on success and 1 on failure.
     * @return Determinant value.
     */
    double (*determinant)(struct matrix *self, int *error);

    /**
     * @brief Return the transpose of the matrix.
     *
     * @param self Matrix instance.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*transpose)(struct matrix *self);

    /**
     * @brief Return the inverse of the matrix.
     *
     * @param self Matrix instance.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*inverse)(struct matrix *self);

    /**
     * @brief Resize the matrix storage.
     *
     * @param self Matrix instance.
     * @param new_rows New row count.
     * @param new_cols New column count.
     * @return 0 on success, -1 on failure.
     */
    int (*resize)(struct matrix *self, int new_rows, int new_cols);

    /**
     * @brief Return a deep copy of the matrix.
     *
     * @param self Matrix instance.
     * @return New matrix instance, or NULL on failure.
     */
    struct matrix *(*copy)(struct matrix *self);

    /**
     * @brief Print the matrix to stdout.
     *
     * @param self Matrix instance.
     */
    void (*print)(const struct matrix *self);

    /**
     * @brief Release the matrix and its internal allocations.
     *
     * @param self Matrix instance.
     */
    void (*free)(struct matrix *self);
} matrix;

/**
 * @brief Public vector object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct vector {
    void *impl;
    void *user_data; /**< User-provided context pointer. */

    /**
     * @brief Read a vector element.
     *
     * @param self Vector instance.
     * @param index Zero-based element index.
     * @return Element value, or 0.0 when out of bounds.
     */
    double (*get)(const struct vector *self, int index);

    /**
     * @brief Write a vector element.
     *
     * @param self Vector instance.
     * @param index Zero-based element index.
     * @param value New element value.
     * @return 0 on success, -1 on invalid index.
     */
    int (*set)(struct vector *self, int index, double value);

    /**
     * @brief Return the element count.
     *
     * @param self Vector instance.
     * @return Number of stored elements.
     */
    int (*size)(const struct vector *self);

    /**
     * @brief Resize the vector.
     *
     * @param self Vector instance.
     * @param new_size New element count.
     * @return 0 on success, -1 on failure.
     */
    int (*resize)(struct vector *self, int new_size);

    /**
     * @brief Return a deep copy of the vector.
     *
     * @param self Vector instance.
     * @return New vector instance, or NULL on failure.
     */
    struct vector *(*copy)(const struct vector *self);

    /**
     * @brief Print the vector to stdout.
     *
     * @param self Vector instance.
     */
    void (*print)(const struct vector *self);

    /**
     * @brief Release the vector and its internal allocations.
     *
     * @param self Vector instance.
     */
    void (*free)(struct vector *self);
} vector;

matrix *create_matrix(int rows, int cols);
vector *create_vector(int size);

/**
 * @brief Add two vectors element-wise and return the result.
 *
 * Uses AVX SIMD instructions when available, otherwise falls back
 * to scalar code.  Both vectors must have the same size.
 *
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return New vector with the element-wise sum, or NULL on failure.
 */
vector *vector_add_simd(vector *a, vector *b);

/**
 * @brief Subtract vector @p b from vector @p a element-wise.
 *
 * Uses AVX SIMD instructions when available, otherwise falls back
 * to scalar code.  Both vectors must have the same size.
 *
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return New vector with the element-wise difference, or NULL on failure.
 */
vector *vector_subtract_simd(vector *a, vector *b);

/**
 * @brief Compute the dot product of two vectors.
 *
 * Uses AVX SIMD instructions when available, otherwise falls back
 * to scalar code.  Both vectors must have the same size.
 *
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return Dot product value, or 0.0 on failure.
 */
double vector_dot_product_simd(vector *a, vector *b);

/**
 * @brief Scale every element of a vector by a scalar and return the result.
 *
 * Uses AVX SIMD instructions when available, otherwise falls back
 * to scalar code.
 *
 * @param v Vector operand.
 * @param scalar Scalar multiplier.
 * @return New scaled vector, or NULL on failure.
 */
vector *vector_scale_simd(vector *v, double scalar);

/**
 * @brief Multiply two matrices using Strassen's algorithm.
 *
 * Uses Strassen's divide-and-conquer algorithm for large matrices
 * (dimensions greater than @c STRASSEN_THRESHOLD, default 64) and
 * falls back to the naive O(n³) method for smaller sub-problems.
 * Non-square or non-power-of-two matrices are padded internally.
 *
 * @param a Left-hand matrix.
 * @param b Right-hand matrix.
 * @return New matrix holding the product, or NULL on failure.
 */
matrix *matrix_multiply_strassen(matrix *a, matrix *b);

#endif // MATRIX_H
