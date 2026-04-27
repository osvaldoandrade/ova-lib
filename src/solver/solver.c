#include "../../include/solver.h"
#include "../matrix/matrix_internal.h"
#include "branch_and_bound.h"
#include "branch_and_cut.h"
#include "lagrangean.h"
#include "simplex.h"
#include "solver_internal.h"

#include <math.h>
#include <stdlib.h>

static int solver_add_constraint(lp_problem *self, const double *coefficients, double bound);
static int solver_set_objective(lp_problem *self, const double *coefficients, ProblemType type);
static int solver_set_bounds(lp_problem *self, const double *bounds);
static int solver_is_feasible(lp_problem *self, vector *solution);
static int solver_improves_objective(lp_problem *self, vector *solution, double old_value, int index);
static int solver_variable_count(const lp_problem *self);
static int solver_constraint_count(const lp_problem *self);
static double solver_solution_value(const lp_problem *self, int index);
static double solver_objective_value(const lp_problem *self);
static ProblemType solver_problem_type(const lp_problem *self);
static void solver_print_problem(lp_problem *self);
static void solver_problem_free(lp_problem *self);

static int solver_dispatch(solver *self, lp_problem *problem, matrix **out_tableau);
static SolverType solver_type_method(const solver *self);
static void solver_free_method(solver *self);
static int solver_ensure_constraint_capacity(lp_problem *self, int min_capacity);

lp_problem *create_problem(int num_variables, int num_constraints) {
    if (num_variables <= 0) {
        return NULL;
    }

    if (num_constraints < 1) {
        num_constraints = 1;
    }

    lp_problem *out = (lp_problem *)calloc(1, sizeof(lp_problem));
    if (!out) {
        return NULL;
    }

    lp_problem_impl *impl = (lp_problem_impl *)calloc(1, sizeof(lp_problem_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->constraints = create_matrix(num_constraints, num_variables);
    impl->objective = create_vector(num_variables);
    impl->bounds = create_vector(num_constraints);
    if (!impl->constraints || !impl->objective || !impl->bounds) {
        if (impl->constraints) {
            impl->constraints->free(impl->constraints);
        }
        if (impl->objective) {
            impl->objective->free(impl->objective);
        }
        if (impl->bounds) {
            impl->bounds->free(impl->bounds);
        }
        free(impl);
        free(out);
        return NULL;
    }

    impl->type = PROBLEM_MAX;
    impl->variable_count = num_variables;
    impl->constraint_count = 0;
    impl->constraint_capacity = num_constraints;
    impl->solution = NULL;
    impl->objective_value = 0.0;

    out->impl = impl;
    out->add_constraint = solver_add_constraint;
    out->set_objective = solver_set_objective;
    out->set_bounds = solver_set_bounds;
    out->is_feasible = solver_is_feasible;
    out->improves_objective = solver_improves_objective;
    out->variable_count = solver_variable_count;
    out->constraint_count = solver_constraint_count;
    out->solution_value = solver_solution_value;
    out->objective_value = solver_objective_value;
    out->problem_type = solver_problem_type;
    out->print = solver_print_problem;
    out->free = solver_problem_free;

    return out;
}

solver *create_solver(SolverType type) {
    solver *out = (solver *)calloc(1, sizeof(solver));
    if (!out) {
        return NULL;
    }

    solver_impl *impl = (solver_impl *)calloc(1, sizeof(solver_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->type = type;
    switch (type) {
        case SOLVER_SIMPLEX:
            impl->solve_fn = simplex_solver;
            break;
        case SOLVER_LAGRANGEAN_SIMPLEX:
            impl->solve_fn = lagrangean_solver;
            break;
        case SOLVER_BRANCH_AND_CUT:
            impl->solve_fn = branch_and_cut_solver;
            break;
        case SOLVER_BRANCH_AND_BOUND:
            impl->solve_fn = branch_and_bound_solver;
            break;
        default:
            free(impl);
            free(out);
            return NULL;
    }

    out->impl = impl;
    out->solve = solver_dispatch;
    out->solver_type = solver_type_method;
    out->free = solver_free_method;
    return out;
}

static int solver_add_constraint(lp_problem *self, const double *coefficients, double bound) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    matrix_impl *constraints = impl ? matrix_impl_from_matrix(impl->constraints) : NULL;
    vector_impl *bounds = impl ? vector_impl_from_vector(impl->bounds) : NULL;
    if (!impl || !constraints || !bounds || !coefficients) {
        return -1;
    }

    if (solver_ensure_constraint_capacity(self, impl->constraint_count + 1) != 0) {
        return -1;
    }

    constraints = matrix_impl_from_matrix(impl->constraints);
    bounds = vector_impl_from_vector(impl->bounds);
    int row = impl->constraint_count;
    for (int i = 0; i < constraints->cols; i++) {
        constraints->data[row][i] = coefficients[i];
    }
    bounds->data[row] = bound;
    impl->constraint_count++;
    return 0;
}

static int solver_set_objective(lp_problem *self, const double *coefficients, ProblemType type) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    vector_impl *objective = impl ? vector_impl_from_vector(impl->objective) : NULL;
    if (!impl || !objective || !coefficients) {
        return -1;
    }

    for (int i = 0; i < objective->size; i++) {
        objective->data[i] = coefficients[i];
    }
    impl->type = type;
    return 0;
}

static int solver_set_bounds(lp_problem *self, const double *bounds) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    vector_impl *bounds_impl = impl ? vector_impl_from_vector(impl->bounds) : NULL;
    if (!impl || !bounds_impl || !bounds) {
        return -1;
    }

    for (int i = 0; i < impl->constraint_count; i++) {
        bounds_impl->data[i] = bounds[i];
    }
    return 0;
}

static int solver_is_feasible(lp_problem *self, vector *solution) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    matrix_impl *constraints = impl ? matrix_impl_from_matrix(impl->constraints) : NULL;
    vector_impl *bounds = impl ? vector_impl_from_vector(impl->bounds) : NULL;
    vector_impl *solution_impl = vector_impl_from_vector(solution);
    if (!impl || !constraints || !bounds || !solution_impl) {
        return 0;
    }

    for (int i = 0; i < impl->constraint_count; i++) {
        double sum = 0.0;
        for (int j = 0; j < constraints->cols; j++) {
            sum += constraints->data[i][j] * solution_impl->data[j];
        }
        if (sum > bounds->data[i]) {
            return 0;
        }
    }
    return 1;
}

static int solver_improves_objective(lp_problem *self, vector *solution, double old_value, int index) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    vector_impl *objective = impl ? vector_impl_from_vector(impl->objective) : NULL;
    vector_impl *solution_impl = vector_impl_from_vector(solution);
    if (!impl || !objective || !solution_impl) {
        return 0;
    }

    double new_obj = 0.0;
    double old_obj = 0.0;
    for (int i = 0; i < solution_impl->size; i++) {
        new_obj += objective->data[i] * solution_impl->data[i];
        old_obj += objective->data[i] * ((i == index) ? old_value : solution_impl->data[i]);
    }
    return (impl->type == PROBLEM_MAX) ? (new_obj > old_obj) : (new_obj < old_obj);
}

static int solver_variable_count(const lp_problem *self) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    return impl ? impl->variable_count : 0;
}

static int solver_constraint_count(const lp_problem *self) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    return impl ? impl->constraint_count : 0;
}

static double solver_solution_value(const lp_problem *self, int index) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    if (!impl || !impl->solution || index < 0 || index >= impl->variable_count) {
        return 0.0;
    }
    return impl->solution[index];
}

static double solver_objective_value(const lp_problem *self) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    return impl ? impl->objective_value : 0.0;
}

static ProblemType solver_problem_type(const lp_problem *self) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    return impl ? impl->type : PROBLEM_MAX;
}

static void solver_print_problem(lp_problem *self) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    matrix_impl *constraints = impl ? matrix_impl_from_matrix(impl->constraints) : NULL;
    vector_impl *objective = impl ? vector_impl_from_vector(impl->objective) : NULL;
    vector_impl *bounds = impl ? vector_impl_from_vector(impl->bounds) : NULL;
    if (!impl || !constraints || !objective || !bounds) {
        return;
    }

    printf("Objective Function (%s):\n", (impl->type == PROBLEM_MAX ? "Maximize" : "Minimize"));
    for (int i = 0; i < objective->size; i++) {
        printf("%f ", objective->data[i]);
    }
    printf("\n\nConstraints (<=):\n");
    for (int i = 0; i < impl->constraint_count; i++) {
        for (int j = 0; j < constraints->cols; j++) {
            printf("%f*x%d ", constraints->data[i][j], j + 1);
            if (j < constraints->cols - 1) {
                printf("+ ");
            }
        }
        printf("<= %f\n", bounds->data[i]);
    }
    printf("\n");
}

static void solver_problem_free(lp_problem *self) {
    if (!self) {
        return;
    }

    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    if (impl) {
        if (impl->constraints) {
            impl->constraints->free(impl->constraints);
        }
        if (impl->objective) {
            impl->objective->free(impl->objective);
        }
        if (impl->bounds) {
            impl->bounds->free(impl->bounds);
        }
        free(impl->solution);
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

static int solver_dispatch(solver *self, lp_problem *problem, matrix **out_tableau) {
    solver_impl *impl = solver_impl_from_public(self);
    if (!impl || !impl->solve_fn) {
        if (out_tableau) {
            *out_tableau = NULL;
        }
        return INFEASIBLE;
    }
    return impl->solve_fn(problem, out_tableau);
}

static SolverType solver_type_method(const solver *self) {
    solver_impl *impl = solver_impl_from_public(self);
    return impl ? impl->type : SOLVER_SIMPLEX;
}

static void solver_free_method(solver *self) {
    if (!self) {
        return;
    }

    free(self->impl);
    self->impl = NULL;
    free(self);
}

static int solver_ensure_constraint_capacity(lp_problem *self, int min_capacity) {
    lp_problem_impl *impl = lp_problem_impl_from_public(self);
    if (!impl || min_capacity <= impl->constraint_capacity) {
        return 0;
    }

    int new_capacity = impl->constraint_capacity;
    while (new_capacity < min_capacity) {
        new_capacity *= 2;
    }

    matrix_impl *constraints = matrix_impl_from_matrix(impl->constraints);
    if (!constraints || impl->constraints->resize(impl->constraints, new_capacity, constraints->cols) != 0) {
        return -1;
    }

    if (impl->bounds->resize(impl->bounds, new_capacity) != 0) {
        return -1;
    }

    impl->constraint_capacity = new_capacity;
    return 0;
}

int is_integer(double value) {
    return fabs(value - round(value)) < 1e-6;
}
