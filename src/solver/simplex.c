#include "simplex.h"
#include <math.h>

int simplex_solver(lp_problem *prob, matrix **out_tableau) {
  if (!out_tableau) {
    return INFEASIBLE;
  }

  *out_tableau = NULL;

  if (!prob || !prob->constraints || !prob->objective || !prob->bounds) {
    return INFEASIBLE;
  }

  int rows = prob->constraint_count;
  int cols = prob->constraints->cols;
  if (rows <= 0 || cols <= 0) {
    return INFEASIBLE;
  }

  *out_tableau = create_matrix(rows + 1, cols + rows + 1);
  if (!*out_tableau) return INFEASIBLE;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      (*out_tableau)->data[i][j] = prob->constraints->data[i][j];
    }
    (*out_tableau)->data[i][cols + i] = 1.0;
    (*out_tableau)->data[i][(*out_tableau)->cols - 1] = prob->bounds->data[i];
  }

  for (int j = 0; j < cols; j++) {
    double coefficient = prob->objective->data[j];
    (*out_tableau)->data[rows][j] = (prob->type == PROBLEM_MIN) ? coefficient : -coefficient;
  }

  while (1) {
    int pivot_col = -1, pivot_row = -1;
    double most_negative = -1e-9;
    double min_ratio = DBL_MAX;

    for (int j = 0; j < cols + rows; j++) {
      double value = (*out_tableau)->data[rows][j];
      if (value < most_negative) {
        most_negative = value;
        pivot_col = j;
      }
    }

    if (pivot_col == -1) {
      if (!prob->solution) {
        prob->solution = calloc((size_t)cols, sizeof(double));
      }
      if (!prob->solution) {
        (*out_tableau)->destroy(*out_tableau);
        *out_tableau = NULL;
        return INFEASIBLE;
      }

      for (int i = 0; i < cols; i++) {
        prob->solution[i] = 0.0;
        for (int r = 0; r < rows; r++) {
          if (fabs((*out_tableau)->data[r][i] - 1.0) < 1e-7) {
            bool is_basic = true;
            for (int rr = 0; rr < rows; rr++) {
              if (rr != r && fabs((*out_tableau)->data[rr][i]) > 1e-7) {
                is_basic = false;
                break;
              }
            }
            if (is_basic) {
              prob->solution[i] = (*out_tableau)->data[r][(*out_tableau)->cols - 1];
              break;
            }
          }
        }
      }
      prob->z_value = (*out_tableau)->data[rows][(*out_tableau)->cols - 1];
      if (prob->type == PROBLEM_MIN) {
        prob->z_value = -prob->z_value;
      }

      return OPTIMAL;
    }

    for (int i = 0; i < rows; i++) {
      double pivot_candidate = (*out_tableau)->data[i][pivot_col];
      if (pivot_candidate > 1e-9) {
        double ratio = (*out_tableau)->data[i][(*out_tableau)->cols - 1] / (*out_tableau)->data[i][pivot_col];
        if (ratio >= 0.0 && ratio < min_ratio) {
          min_ratio = ratio;
          pivot_row = i;
        }
      }
    }

    if (pivot_row == -1) {
      return UNBOUNDED;
    }

    double pivot_value = (*out_tableau)->data[pivot_row][pivot_col];
    if (fabs(pivot_value) < 1e-12) {
      (*out_tableau)->destroy(*out_tableau);
      *out_tableau = NULL;
      return INFEASIBLE;
    }

    for (int j = 0; j < (*out_tableau)->cols; j++) {
      (*out_tableau)->data[pivot_row][j] /= pivot_value;
    }

    for (int i = 0; i <= rows; i++) {
      if (i != pivot_row) {
        double factor = (*out_tableau)->data[i][pivot_col];
        for (int j = 0; j < (*out_tableau)->cols; j++) {
          (*out_tableau)->data[i][j] -= (*out_tableau)->data[pivot_row][j] * factor;
        }
      }
    }
  }

  return INFEASIBLE;
}
