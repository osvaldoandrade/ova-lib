#ifndef BRANCH_AND_BOUND_H
#define BRANCH_AND_BOUND_H

#include "../../include/solver.h"
#include "solver_internal.h"

int branch_and_bound_solver(lp_problem *prob, matrix **out_tableau);

#endif // BRANCH_AND_BOUND_H
