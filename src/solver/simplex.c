/**
 * @file simplex.c
 * @brief Simplex algorithm implementation.
 */

#include "simplex.h"
#include <math.h>

/**
 * @brief Solve a linear programming problem using the simplex method.
 */
int simplex_solver(lp_problem *prob, matrix **out_tableau) {
  if (!prob || !prob->constraints || !prob->objective || !prob->bounds) {
    return INFEASIBLE;
  }

  int rows = prob->constraints->rows;
  int cols = prob->constraints->cols;
  *out_tableau = create_matrix(rows + 1, cols + rows + 1);
  if (!*out_tableau) return INFEASIBLE;

  // Initialize the tableau with constraints and slack variables directly in out_tableau
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      (*out_tableau)->data[i][j] = prob->constraints->data[i][j];
    }
    (*out_tableau)->data[i][cols + i] = 1;  // Adding slack variable
    (*out_tableau)->data[i][(*out_tableau)->cols - 1] = prob->bounds->data[i];  // Setting RHS values
  }

  // Setting up the objective function in the last row of out_tableau
  for (int j = 0; j < cols; j++) {
    (*out_tableau)->data[rows][j] = -prob->objective->data[j];  // Objective function needs to be maximized
  }

  printf("Tableau after initialization:\n");
  (*out_tableau)->print(*out_tableau);
  printf("\n");

  while (1) {
    int pivot_col = -1, pivot_row = -1;
    double min_ratio = DBL_MAX;

    // Identifying the pivot column (most negative coefficient in the objective function row)
    for (int j = 0; j < cols + rows; j++) {
      if ((*out_tableau)->data[rows][j] < 0) {
        pivot_col = j;
        break;
      }
    }

    if (pivot_col == -1) {
      printf("Tableau at optimality:\n");
      (*out_tableau)->print(*out_tableau);
      printf("\n");

      // Store the solution values into the lp_problem structure
      if (!prob->solution) {
        prob->solution = malloc(sizeof(double) * cols);
      }
      for (int i = 0; i < cols; i++) {
        prob->solution[i] = 0; // Initialize with zero
        for (int r = 0; r < rows; r++) {
          if (fabs((*out_tableau)->data[r][i] - 1.0) < 1e-5) { // Check for the basic variable indicator
            bool isBasic = true;
            for (int c = 0; c < rows; c++) {
              if (c != r && fabs((*out_tableau)->data[c][i]) > 1e-5) {
                isBasic = false;
                break;
              }
            }
            if (isBasic) {
              prob->solution[i] = (*out_tableau)->data[r][(*out_tableau)->cols - 1];
              break;
            }
          }
        }
      }
      prob->z_value = (*out_tableau)->data[rows][(*out_tableau)->cols - 1]; // Store the objective value

      return OPTIMAL;  // No negative coefficients, optimal solution found
    }

    // Minimum ratio test (Bland's Rule) to find the pivot row
    for (int i = 0; i < rows; i++) {
      if ((*out_tableau)->data[i][pivot_col] > 0) {
        double ratio = (*out_tableau)->data[i][(*out_tableau)->cols - 1] / (*out_tableau)->data[i][pivot_col];
        if (ratio < min_ratio) {
          min_ratio = ratio;
          pivot_row = i;
        }
      }
    }

    if (pivot_row == -1) {
      printf("Tableau when unbounded:\n");
      (*out_tableau)->print(*out_tableau);
      printf("\n");
      return UNBOUNDED;  // No valid pivot found, solution is unbounded
    }

    // Pivoting: Normalize the pivot row and zero out the rest of the pivot column
    double pivot_value = (*out_tableau)->data[pivot_row][pivot_col];
    for (int j = 0; j < (*out_tableau)->cols; j++) {
      (*out_tableau)->data[pivot_row][j] /= pivot_value;  // Normalize the pivot row
    }

    for (int i = 0; i <= rows; i++) {
      if (i != pivot_row) {
        double factor = (*out_tableau)->data[i][pivot_col];
        for (int j = 0; j < (*out_tableau)->cols; j++) {
          (*out_tableau)->data[i][j] -= (*out_tableau)->data[pivot_row][j] * factor;
        }
      }
    }

    printf("Tableau after pivoting around row %d, column %d:\n", pivot_row, pivot_col);
    (*out_tableau)->print(*out_tableau);
    printf("\n");
  }

  // Should not reach here, but in case the loop is exited unexpectedly
  return INFEASIBLE;
}
