#ifndef LAGRANGEAN_H
#define LAGRANGEAN_H

#include "../../include/solver.h"
#include "solver_internal.h"

int lagrangean_solver(lp_problem *prob, matrix **out_tableau);

#endif // LAGRANGEAN_H
