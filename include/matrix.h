/**
 * @file matrix.h
 * @brief Matrix and vector utilities.
 */

#ifndef LINEAR_H
#define LINEAR_H

#include "types.h"
#include <stddef.h>

/**
 * @brief The struct represents a matrix.
 */
typedef struct matrix {
    double **data;
    int rows;
    int cols;

    /**
     * @brief Adds the elements of the given matrix with the elements of another matrix.
     *
     * This function creates a new matrix by adding the corresponding elements of the
     * given matrix (self) with the elements of the other matrix. The new matrix is
     * created and returned.
     *
     * @param self The matrix to which the elements will be added.
     * @param other The matrix from which the elements will be added.
     * @return A new matrix containing the result of the addition.
     */
    struct matrix *(*add)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Subtract the given matrix from the current matrix.
     *
     * This function subtracts the elements of the given matrix (`other`) from the elements
     * of the current matrix (`self`), and returns the resulting matrix.
     * Both matrices must have the same dimensions for the subtraction to be valid.
     *
     * @param self The current matrix.
     * @param other The matrix to be subtracted from the current matrix.
     * @return A pointer to the resulting matrix if successful, or `NULL` if an error occurred.
     */
    struct matrix *(*subtract)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Multiplies the matrix with another matrix.
     *
     * This function multiplies the matrix with another matrix and returns the result.
     * The result is a new dynamically allocated matrix, so remember to free the memory
     * when you are done using it.
     *
     * @param self The matrix on which the function is called.
     * @param other The matrix to multiply with.
     * @return struct matrix* The resulting matrix.
     */
    struct matrix *(*multiply)(struct matrix *self, const struct matrix *other);

    /**
     * @brief Calculates the determinant of the matrix.
     *
     * This function calculates the determinant of the matrix. The determinant is a scalar
     * value that represents a specific property of the matrix. The function returns the
     * determinant as a double value.
     *
     * @param self The matrix for which the determinant will be calculated.
     * @param error Pointer to an integer that will indicate if an error occurred during the calculation.
     * @return The determinant of the matrix as a double value.
     */
    double (*determinant)(struct matrix *self, int *error);

    /**
     * @brief Transposes the matrix.
     *
     * This function transposes the given matrix and returns the transposed matrix.
     * The transposed matrix is a new dynamically allocated matrix, so remember to
     * free the memory when you are done using it.
     *
     * @param self The matrix to be transposed.
     * @return The transposed matrix.
     */
    struct matrix *(*transpose)(struct matrix *self);

    /**
     * @brief Calculates the inverse of the matrix.
     *
     * This function calculates the inverse of the given matrix and returns the inverse matrix.
     * The inverse matrix is a new dynamically allocated matrix, so remember to free the memory
     * when you are done using it.
     *
     * @param self The matrix for which the inverse will be calculated.
     * @return The inverse matrix.
     */
    struct matrix *(*inverse)(struct matrix *self);

    /**
     * @brief Function pointer to print the matrix.
     *
     * This function pointer is used to print the matrix. It takes a pointer to a `struct matrix` object and prints its elements.
     *
     * @param self A pointer to the `struct matrix` object to be printed.
     */
    void (*print)(const struct matrix *self);

    /**
     * @brief Function pointer to destroy a matrix object.
     *
     * This function pointer is used to destroy a matrix object.
     * It takes a pointer to a matrix object and frees the memory
     * allocated for the matrix data and the matrix object itself.
     *
     * @param self A pointer to the matrix object to be destroyed.
     */
    void (*destroy)(struct matrix *self);

  /**
    * @brief Function pointer to resize the matrix.
    *
    * This function pointer is used to resize the matrix, changing the number of rows and columns.
    *
    * @param self A pointer to the matrix object to be resized.
    * @param newRows The new number of rows for the matrix.
    * @param newCols The new number of columns for the matrix.
    */
    void (*resize)(struct matrix *self, int newRows, int newCols);

    struct matrix* (*copy)(struct matrix *self);
} matrix;

/**
 * @struct The struct represents a vector.
 */
typedef struct vector {
    double *data;
    int size;
    void (*resize)(struct vector *self, int newRows);
    void (*print)(const struct vector *self);
    void (*destroy)(struct vector *self);
} vector;

/**
 * @brief Creates a matrix with the specified number of rows and columns.
 *
 * This function dynamically allocates memory for the matrix structure and its data, initializes the function pointers for matrix operations,
 * and returns a pointer to the created matrix. The memory must be released by calling the \c destroy function when no longer needed.
 *
 * @param rows The number of rows in the matrix. Must be a positive integer.
 * @param cols The number of columns in the matrix. Must be a positive integer.
 * @return A pointer to the created matrix, or \c NULL if memory allocation fails or the specified dimensions are non-positive.
 */
matrix *create_matrix(int rows, int cols);

/**
 * @brief Creates a new vector with the specified size.
 *
 * This function dynamically allocates memory for a new vector and initializes it
 * with a given size.
 *
 * @param size The size of the vector to be created.
 * @return A pointer to the newly created vector.
 */
vector *create_vector(int size);

#endif // LINEAR_H
