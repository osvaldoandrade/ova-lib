#include "../../include/linear.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

matrix *matrix_add(matrix *self, const matrix *other);
matrix *matrix_subtract(matrix *self, const matrix *other);
matrix *matrix_multiply(matrix *self, const matrix *other);
double matrix_determinant(matrix *self, int *error);
matrix *matrix_transpose(matrix *self);
matrix *matrix_inverse(matrix *self);
void destroy_matrix(matrix *m);
void print_matrix(const matrix *m);

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
    m->print = print_matrix;
    m->destroy = destroy_matrix;

    return m;
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

    destroy_matrix(temp);
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
        destroy_matrix(inverse);
        destroy_matrix(augmented);
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
                destroy_matrix(inverse);
                destroy_matrix(augmented);
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

    destroy_matrix(augmented);
    return inverse;
}

void destroy_matrix(matrix *m) {
    if (m != NULL) {
        if (m->data != NULL) {
            for (int i = 0; i < m->rows; i++) {
                free(m->data[i]);  // Free each row
            }
            free(m->data);
        }
        free(m);
    }
}

void print_matrix(const matrix *m) {
    if (m == NULL) {
        printf("Matrix is NULL\n");
        return;
    }
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            printf("%9.3f ", m->data[i][j]);  // Adjust formatting as necessary
        }
        printf("\n");
    }
}

static int simplex(lp_problem *prob, matrix **out_tableau) {
    if (!prob || !prob->constraints || !prob->objective || !prob->bounds) return INFEASIBLE;

    int rows = prob->constraints->rows;
    int cols = prob->constraints->cols;
    matrix *tableau = create_matrix(rows + 1, cols + rows + 1);

    if (!tableau) return INFEASIBLE;

    // Initialize the tableau with constraints and slack variables
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            tableau->data[i][j] = prob->constraints->data[i][j];
        }
        tableau->data[i][cols + i] = 1;  // Slack variable
        tableau->data[i][tableau->cols - 1] = prob->bounds->data[i];  // Right-hand side
    }

    // Initialize the objective function row
    for (int j = 0; j < cols; j++) {
        tableau->data[rows][j] = -prob->objective->data[j];
    }

    while (1) {
        int pivot_col = -1, pivot_row = -1;
        double min_ratio = DBL_MAX;

        // Find the most negative coefficient in the last row
        for (int j = 0; j < cols + rows; j++) {
            if (tableau->data[rows][j] < 0) {
                pivot_col = j;
                break;
            }
        }

        if (pivot_col == -1) {
            *out_tableau = tableau;
            return OPTIMAL;  // No negative coefficients, solution is optimal
        }

        // Minimum ratio test to find the leaving row
        for (int i = 0; i < rows; i++) {
            if (tableau->data[i][pivot_col] > 0) {
                double ratio = tableau->data[i][tableau->cols - 1] / tableau->data[i][pivot_col];
                if (ratio < min_ratio) {
                    min_ratio = ratio;
                    pivot_row = i;
                }
            }
        }

        if (pivot_row == -1) {
            destroy_matrix(tableau);
            return UNBOUNDED;  // No valid pivot row found, solution is unbounded
        }

        // Pivot operation
        double pivot_value = tableau->data[pivot_row][pivot_col];
        for (int j = 0; j < tableau->cols; j++) {
            tableau->data[pivot_row][j] /= pivot_value;
        }

        // Zero out other entries in the pivot column
        for (int i = 0; i <= rows; i++) {
            if (i != pivot_row) {
                double factor = tableau->data[i][pivot_col];
                for (int j = 0; j < tableau->cols; j++) {
                    tableau->data[i][j] -= tableau->data[pivot_row][j] * factor;
                }
            }
        }
    }
}

static matrix *lagrange_relaxation(lp_problem *prob, double lambda, solver_func solve) {
    if (!prob || !prob->constraints || !prob->objective || !prob->bounds || lambda < 0 || !solve) return NULL;

    // Copy original problem to modify objective function with Lagrange multipliers
    lp_problem *modified_prob = malloc(sizeof(lp_problem));
    if (!modified_prob) return NULL;

    *modified_prob = *prob;  // Shallow copy to start with

    // Adjust the objective function by subtracting lambda * constraints
    modified_prob->objective = create_vector(prob->objective->size);
    if (!modified_prob->objective) {
        free(modified_prob);
        return NULL;
    }

    for (int j = 0; j < prob->objective->size; j++) {
        modified_prob->objective->data[j] = prob->objective->data[j];
        for (int i = 0; i < prob->constraints->rows; i++) {
            modified_prob->objective->data[j] -= lambda * prob->constraints->data[i][j];
        }
    }

    // Use the provided solver function to optimize the relaxed problem
    matrix *solution = solve(modified_prob);

    // Clean up the modified problem if needed
    destroy_vector(modified_prob->objective);
    free(modified_prob);

    return solution;
}

static vector *primal_heuristic(lp_problem *prob) {
    if (!prob || !prob->constraints || !prob->objective || !prob->bounds) return NULL;

    // Create a vector to hold the initial feasible solution
    vector *solution = create_vector(prob->objective->size);
    if (!solution) return NULL;

    // Initialize solution with lower bounds or zero if no bounds are given
    for (int i = 0; i < solution->size; i++) {
        solution->data[i] = (prob->bounds->data[i] > 0) ? prob->bounds->data[i] : 0;
    }

    // Try to improve the solution by increasing each variable and checking feasibility
    int improving = 1;
    while (improving) {
        improving = 0;
        for (int i = 0; i < solution->size; i++) {
            double old_value = solution->data[i];
            solution->data[i] += 1.0;  // Increment the variable to improve the objective

            if (is_feasible(prob, solution)) {
                if (!improves_objective(prob, solution, old_value, i)) {
                    solution->data[i] = old_value;  // Revert if not improving
                } else {
                    improving = 1;  // Mark as improved
                }
            } else {
                solution->data[i] = old_value;  // Revert if not feasible
            }
        }
    }

    return solution;
}

static int is_feasible(lp_problem *prob, vector *solution) {
    for (int i = 0; i < prob->constraints->rows; i++) {
        double sum = 0;
        for (int j = 0; j < prob->constraints->cols; j++) {
            sum += prob->constraints->data[i][j] * solution->data[j];
        }
        if (sum > prob->bounds->data[i]) return 0;  // Constraint violated
    }
    return 1;  // All constraints satisfied
}

static int improves_objective(lp_problem *prob, vector *solution, double old_value, int index) {
    double new_obj = 0, old_obj = 0;
    for (int i = 0; i < solution->size; i++) {
        new_obj += prob->objective->data[i] * solution->data[i];
        old_obj += prob->objective->data[i] * ((i == index) ? old_value : solution->data[i]);
    }
    // Compare strings correctly
    return (prob->type == PROBLEM_MAX) ? (new_obj > old_obj) : (new_obj < old_obj);
}





