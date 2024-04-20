#ifndef LINEAR_H
#define LINEAR_H

#include <stddef.h>

#define OPTIMAL 0
#define UNBOUNDED -1
#define INFEASIBLE -2

typedef enum {
    PROBLEM_MAX,
    PROBLEM_MIN,
} ProblemType;

typedef struct matrix {
    double **data;
    int rows;
    int cols;
    struct matrix *(*add)(struct matrix *self, const struct matrix *other);
    struct matrix *(*subtract)(struct matrix *self, const struct matrix *other);
    struct matrix *(*multiply)(struct matrix *self, const struct matrix *other);
    double (*determinant)(struct matrix *self, int *error);
    struct matrix *(*transpose)(struct matrix *self);
    struct matrix *(*inverse)(struct matrix *self);
    void (*print)(const struct matrix *self);
    void (*destroy)(struct matrix *self);
} matrix;

typedef struct vector {
    double *data;
    int size;
} vector;

// Defines a Linear Programming problem with continuous variables
typedef struct lp_problem {
    matrix *constraints;
    vector *objective;
    vector *bounds;
    ProblemType type;
} lp_problem;

typedef matrix* (*solver_func)(lp_problem*);


matrix *create_matrix(int rows, int cols);


vector *create_vector(int size);
void destroy_vector(vector *v);
void print_vector(const vector *v);

static int simplex(lp_problem *prob, matrix **out_tableau);
static matrix *lagrange_relaxation(lp_problem *prob, double lambda, solver_func solve);
static int is_feasible(lp_problem *prob, vector *solution);
static int improves_objective(lp_problem *prob, vector *solution, double old_value, int index);



#endif // LINEAR_H
