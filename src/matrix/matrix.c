#include "../../include/matrix.h"

#include <float.h>
#include <string.h>

void vector_resize(vector *self, int newSize);
void vector_print(const vector *self);
void vector_destroy(vector *self);

matrix *matrix_add(matrix *self, const matrix *other);
matrix *matrix_subtract(matrix *self, const matrix *other);
matrix *matrix_multiply(matrix *self, const matrix *other);
double matrix_determinant(matrix *self, int *error);
matrix *matrix_transpose(matrix *self);
matrix *matrix_inverse(matrix *self);
void matrix_resize(matrix *self, int newRows, int newCols);
matrix *matrix_copy(matrix *self);
void matrix_destroy(matrix *self);
void matrix_print(const matrix *self);

matrix *create_matrix(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        return NULL;  // Handle non-positive dimensions
    }

    matrix *m = malloc(sizeof(matrix));
    if (m == NULL) {
        return NULL;  // Handle memory allocation failure for the matrix structure
    }

    m->data = malloc(rows * sizeof(double *));
    if (m->data == NULL) {
        free(m);  // Free the allocated structure if row allocation fails
        return NULL;
    }

    for (int i = 0; i < rows; i++) {
        m->data[i] = malloc(cols * sizeof(double));
        if (m->data[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(m->data[j]);  // Free previously allocated rows on failure
            }
            free(m->data);
            free(m);
            return NULL;
        }
    }

    m->rows = rows;
    m->cols = cols;

    // Set the function pointers for matrix operations
    m->add = matrix_add;
    m->subtract = matrix_subtract;
    m->multiply = matrix_multiply;
    m->determinant = matrix_determinant;
    m->transpose = matrix_transpose;
    m->inverse = matrix_inverse;
    m->resize = matrix_resize;
    m->copy = matrix_copy;
    m->print = matrix_print;
    m->destroy = matrix_destroy;

    return m;
}

vector *create_vector(int size) {
  vector *v = malloc(sizeof(vector));
  if (v == NULL) {
    return NULL; // Memory allocation failed
  }

  v->data = calloc(size, sizeof(double)); // Use calloc to initialize elements to 0
  if (v->data == NULL) {
    free(v); // Clean up vector allocation if data allocation fails
    return NULL;
  }

  v->size = size;
  v->resize = vector_resize;
  v->print = vector_print;
  v->destroy = vector_destroy;

  return v;
}

void vector_resize(vector *self, int newSize) {
  if (self == NULL) return;

  double *newData = realloc(self->data, newSize * sizeof(double));
  if (newData == NULL) return; // Handle realloc failure

  // Initialize new elements to zero if size is increased
  if (newSize > self->size) {
    for (int i = self->size; i < newSize; i++) {
      newData[i] = 0.0;
    }
  }

  self->data = newData;
  self->size = newSize;
}

void vector_print(const vector *self) {
  if (self == NULL) {
    printf("Vector is NULL\n");
    return;
  }

  printf("[");
  for (int i = 0; i < self->size; i++) {
    printf("%f", self->data[i]);
    if (i < self->size - 1) printf(", ");
  }
  printf("]\n");
}

void vector_destroy(vector *self) {
  if (self != NULL) {
    free(self->data); // Free the data array
    free(self); // Free the vector structure
  }
}

matrix *matrix_add(matrix *self, const matrix *other) {
    if (self == NULL || other == NULL || self->rows != other->rows || self->cols != other->cols) {
        return NULL;
    }

    matrix *result = create_matrix(self->rows, self->cols);
    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < self->cols; j++) {
            result->data[i][j] = self->data[i][j] + other->data[i][j];
        }
    }

    return result;
}

matrix *matrix_subtract(matrix *self, const matrix *other) {
    if (self == NULL || other == NULL || self->rows != other->rows || self->cols != other->cols) {
        return NULL; // Error handling for invalid input or size mismatch
    }

    matrix *result = create_matrix(self->rows, self->cols);
    if (result == NULL) {
        return NULL; // Error handling for memory allocation failure
    }

    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < self->cols; j++) {
            result->data[i][j] = self->data[i][j] - other->data[i][j];
        }
    }

    return result;
}

matrix *matrix_multiply(matrix *self, const matrix *other) {
    if (self == NULL || other == NULL || self->cols != other->rows) {
        return NULL; // Error handling for invalid input or size mismatch
    }

    matrix *result = create_matrix(self->rows, other->cols);
    if (result == NULL) {
        return NULL; // Error handling for memory allocation failure
    }

    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < other->cols; j++) {
            result->data[i][j] = 0; // Initialize element
            for (int k = 0; k < self->cols; k++) {
                result->data[i][j] += self->data[i][k] * other->data[k][j];
            }
        }
    }

    return result;
}

double matrix_determinant(matrix *self, int *error) {
    if (self == NULL || self->rows != self->cols) {
        if (error) *error = 1;  // Set error flag
        return 0;  // Return zero or an appropriate error value
    }

    int n = self->rows;
    double det = 1.0;
    matrix *temp = create_matrix(n, n);
    if (!temp) {
        if (error) *error = 1;
        return 0;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            temp->data[i][j] = self->data[i][j];
        }
    }

    for (int i = 0; i < n; i++) {
        if (temp->data[i][i] == 0) {
            int row_swapped = 0;
            for (int k = i + 1; k < n; k++) {
                if (temp->data[k][i] != 0) {
                    for (int j = 0; j < n; j++) {
                        double swap = temp->data[i][j];
                        temp->data[i][j] = temp->data[k][j];
                        temp->data[k][j] = swap;
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
            double factor = temp->data[k][i] / temp->data[i][i];
            for (int j = i; j < n; j++) {
                temp->data[k][j] -= temp->data[i][j] * factor;
            }
        }

        det *= temp->data[i][i];
    }

  matrix_destroy(temp);
    if (error) *error = 0;  // Reset error flag
    return det;
}

matrix *matrix_transpose(matrix *self) {
    if (self == NULL) {
        return NULL;  // Error handling for NULL input
    }

    // Create a new matrix with flipped dimensions
    matrix *result = create_matrix(self->cols, self->rows);
    if (result == NULL) {
        return NULL;  // Error handling for memory allocation failure
    }

    // Assign values to the new matrix based on transpose rules
    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < self->cols; j++) {
            result->data[j][i] = self->data[i][j];
        }
    }

    return result;
}

matrix *matrix_inverse(matrix *self) {
    if (self == NULL || self->rows != self->cols) {
        return NULL;  // Error handling for invalid input
    }

    int n = self->rows;
    matrix *inverse = create_matrix(n, n);
    matrix *augmented = create_matrix(n, n * 2);

    if (inverse == NULL || augmented == NULL) {
      matrix_destroy(inverse);
      matrix_destroy(augmented);
        return NULL;  // Memory allocation failure
    }

    // Initialize the augmented matrix [self | I]
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            augmented->data[i][j] = self->data[i][j];
            augmented->data[i][j + n] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Perform Gaussian elimination to get the form [I | self^-1]
    for (int i = 0; i < n; i++) {
        // Ensure the pivot is non-zero
        if (augmented->data[i][i] == 0) {
            // Try to find a non-zero pivot and swap rows
            int found = 0;
            for (int k = i + 1; k < n; k++) {
                if (augmented->data[k][i] != 0) {
                    // Swap rows
                    for (int j = 0; j < 2 * n; j++) {
                        double temp = augmented->data[i][j];
                        augmented->data[i][j] = augmented->data[k][j];
                        augmented->data[k][j] = temp;
                    }
                    found = 1;
                    break;
                }
            }
            if (!found) {
              matrix_destroy(inverse);
              matrix_destroy(augmented);
                return NULL;  // Matrix is singular, no inverse exists
            }
        }

        // Scale the pivot row to make the pivot element 1
        double pivot = augmented->data[i][i];
        for (int j = 0; j < 2 * n; j++) {
            augmented->data[i][j] /= pivot;
        }

        // Eliminate all other entries in the current column
        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = augmented->data[k][i];
                for (int j = 0; j < 2 * n; j++) {
                    augmented->data[k][j] -= factor * augmented->data[i][j];
                }
            }
        }
    }

    // Extract the inverse from the augmented matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inverse->data[i][j] = augmented->data[i][j + n];
        }
    }

  matrix_destroy(augmented);
    return inverse;
}

void matrix_resize(matrix *self, int newRows, int newCols) {
  if (self == NULL) return;

  // Free rows that will be discarded if the matrix is shrinking
  for (int i = newRows; i < self->rows; i++) {
    free(self->data[i]);
  }

  // Resize rows pointer array
  double **newData = realloc(self->data, newRows * sizeof(double *));
  if (newData == NULL) return; // Handle realloc failure

  self->data = newData;

  // Adjust each row
  for (int i = 0; i < newRows; i++) {
    if (i < self->rows) {
      // Resize existing rows
      double *newRow = realloc(self->data[i], newCols * sizeof(double));
      if (newRow == NULL) return; // Handle realloc failure
      self->data[i] = newRow;
    } else {
      // Initialize new rows
      self->data[i] = malloc(newCols * sizeof(double));
      if (self->data[i] == NULL) return; // Handle malloc failure
      // Initialize the new elements to zero
      for (int j = 0; j < newCols; j++) {
        self->data[i][j] = 0.0;
      }
    }
  }

  // Update dimensions
  self->rows = newRows;
  self->cols = newCols;
}

matrix *matrix_copy(matrix *self) {
  if (!self) return NULL;

  matrix* copy = create_matrix(self->rows, self->cols);
  if (!copy) return NULL;

  for (int i = 0; i < self->rows; i++) {
    memcpy(copy->data[i], self->data[i], self->cols * sizeof(double));
  }

  return copy;
}

void matrix_destroy(matrix *self) {
    if (self != NULL) {
        if (self->data != NULL) {
            for (int i = 0; i < self->rows; i++) {
                free(self->data[i]);  // Free each row
            }
            free(self->data);
        }
        free(self);
    }
}

void matrix_print(const matrix *self) {
    if (self == NULL) {
        printf("Matrix is NULL\n");
        return;
    }
    for (int i = 0; i < self->rows; i++) {
        for (int j = 0; j < self->cols; j++) {
            printf("%9.3f ", self->data[i][j]);  // Adjust formatting as necessary
        }
        printf("\n");
    }
}


