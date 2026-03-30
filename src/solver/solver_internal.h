#ifndef SOLVER_INTERNAL_H
#define SOLVER_INTERNAL_H

#include "../../include/solver.h"

typedef struct lp_problem_impl {
    matrix *constraints;
    vector *objective;
    vector *bounds;
    ProblemType type;
    int variable_count;
    int constraint_count;
    int constraint_capacity;
    double *solution;
    double objective_value;
} lp_problem_impl;

typedef struct solver_impl {
    SolverType type;
    int (*solve_fn)(lp_problem *problem, matrix **out_tableau);
} solver_impl;

static inline lp_problem_impl *lp_problem_impl_from_public(const lp_problem *problem) {
    return problem ? (lp_problem_impl *)problem->impl : NULL;
}

static inline solver_impl *solver_impl_from_public(const solver *s) {
    return s ? (solver_impl *)s->impl : NULL;
}

#endif // SOLVER_INTERNAL_H
