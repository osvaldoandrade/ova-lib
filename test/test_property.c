#include "base_test.h"
#include "property_test.h"
#include "../include/list.h"
#include "../include/sort.h"
#include "../include/set.h"
#include "../include/matrix.h"

#include <stdint.h>
#include <string.h>
#include <tgmath.h>

#define FLOAT_TOL 0.0001

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */

static int int_cmp(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int int_hash(void *key, int capacity) {
    if (!key || capacity <= 0) {
        return 0;
    }
    uint32_t x = (uint32_t)(*(int *)key);
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return (int)(x % (uint32_t)capacity);
}

/* ------------------------------------------------------------------ */
/*  Property: list insert-then-get returns the same element           */
/* ------------------------------------------------------------------ */

static int prop_list_insert_get(int iteration) {
    (void)iteration;
    int n = pbt_rand_int(1, PBT_MAX_COLLECTION_SIZE);
    int *values = malloc((size_t)n * sizeof(int));
    if (!values) { return 0; }

    list *lst = create_list(ARRAY_LIST, n, NULL);
    if (!lst) { free(values); return 0; }

    for (int i = 0; i < n; i++) {
        values[i] = pbt_rand_int(-1000, 1000);
        lst->insert(lst, &values[i], i);
    }

    int ok = 1;
    for (int i = 0; i < n; i++) {
        int *retrieved = (int *)lst->get(lst, i);
        if (!retrieved || *retrieved != values[i]) {
            ok = 0;
            break;
        }
    }

    lst->free(lst);
    free(values);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Property: sorting produces a non-decreasing sequence              */
/* ------------------------------------------------------------------ */

static int prop_sort_non_decreasing(int iteration) {
    (void)iteration;
    int n = pbt_rand_int(0, PBT_MAX_COLLECTION_SIZE);
    int *values = malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    if (!values) { return 0; }

    list *lst = create_list(ARRAY_LIST, n > 0 ? n : 1, NULL);
    if (!lst) { free(values); return 0; }

    for (int i = 0; i < n; i++) {
        values[i] = pbt_rand_int(-10000, 10000);
        lst->insert(lst, &values[i], i);
    }

    sorter *s = create_sorter(int_cmp);
    if (!s) { lst->free(lst); free(values); return 0; }

    s->sort(s, lst);

    int ok = 1;
    for (int i = 1; i < lst->size(lst); i++) {
        int *prev = (int *)lst->get(lst, i - 1);
        int *cur  = (int *)lst->get(lst, i);
        if (*prev > *cur) {
            ok = 0;
            break;
        }
    }

    s->free(s);
    lst->free(lst);
    free(values);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Property: set union is commutative  (A ∪ B == B ∪ A)             */
/* ------------------------------------------------------------------ */

static int sets_equal(const set *a, const set *b) {
    if (a->size(a) != b->size(b)) {
        return 0;
    }
    list *la = a->to_list(a);
    if (!la) { return 0; }
    int ok = 1;
    for (int i = 0; i < la->size(la); i++) {
        if (!b->contains(b, la->get(la, i))) {
            ok = 0;
            break;
        }
    }
    la->free(la);
    return ok;
}

static int prop_set_union_commutative(int iteration) {
    (void)iteration;
    int na = pbt_rand_int(0, 20);
    int nb = pbt_rand_int(0, 20);
    int *va = malloc((size_t)(na > 0 ? na : 1) * sizeof(int));
    int *vb = malloc((size_t)(nb > 0 ? nb : 1) * sizeof(int));
    if (!va || !vb) { free(va); free(vb); return 0; }

    set *a = create_set(SET_HASH, int_cmp, int_hash);
    set *b = create_set(SET_HASH, int_cmp, int_hash);
    if (!a || !b) {
        if (a) { a->free(a); } if (b) { b->free(b); }
        free(va); free(vb);
        return 0;
    }

    for (int i = 0; i < na; i++) {
        va[i] = pbt_rand_int(-100, 100);
        a->add(a, &va[i]);
    }
    for (int i = 0; i < nb; i++) {
        vb[i] = pbt_rand_int(-100, 100);
        b->add(b, &vb[i]);
    }

    set *ab = a->union_with(a, b);
    set *ba = b->union_with(b, a);

    int ok = 0;
    if (ab && ba) {
        ok = sets_equal(ab, ba);
    }

    if (ab) { ab->free(ab); }
    if (ba) { ba->free(ba); }
    a->free(a);
    b->free(b);
    free(va);
    free(vb);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Property: set union is associative  ((A ∪ B) ∪ C == A ∪ (B ∪ C)) */
/* ------------------------------------------------------------------ */

static int prop_set_union_associative(int iteration) {
    (void)iteration;
    int na = pbt_rand_int(0, 15);
    int nb = pbt_rand_int(0, 15);
    int nc = pbt_rand_int(0, 15);
    int *va = malloc((size_t)(na > 0 ? na : 1) * sizeof(int));
    int *vb = malloc((size_t)(nb > 0 ? nb : 1) * sizeof(int));
    int *vc = malloc((size_t)(nc > 0 ? nc : 1) * sizeof(int));
    if (!va || !vb || !vc) { free(va); free(vb); free(vc); return 0; }

    set *a = create_set(SET_HASH, int_cmp, int_hash);
    set *b = create_set(SET_HASH, int_cmp, int_hash);
    set *c = create_set(SET_HASH, int_cmp, int_hash);
    if (!a || !b || !c) {
        if (a) { a->free(a); } if (b) { b->free(b); } if (c) { c->free(c); }
        free(va); free(vb); free(vc);
        return 0;
    }

    for (int i = 0; i < na; i++) { va[i] = pbt_rand_int(-50, 50); a->add(a, &va[i]); }
    for (int i = 0; i < nb; i++) { vb[i] = pbt_rand_int(-50, 50); b->add(b, &vb[i]); }
    for (int i = 0; i < nc; i++) { vc[i] = pbt_rand_int(-50, 50); c->add(c, &vc[i]); }

    set *ab = a->union_with(a, b);
    set *ab_c = ab ? ab->union_with(ab, c) : NULL;

    set *bc = b->union_with(b, c);
    set *a_bc = bc ? a->union_with(a, bc) : NULL;

    int ok = 0;
    if (ab_c && a_bc) {
        ok = sets_equal(ab_c, a_bc);
    }

    if (ab) { ab->free(ab); }
    if (ab_c) { ab_c->free(ab_c); }
    if (bc) { bc->free(bc); }
    if (a_bc) { a_bc->free(a_bc); }
    a->free(a);
    b->free(b);
    c->free(c);
    free(va);
    free(vb);
    free(vc);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Property: matrix multiplication is associative  (AB)C == A(BC)    */
/* ------------------------------------------------------------------ */

static int matrices_approx_equal(matrix *m1, matrix *m2) {
    if (m1->rows(m1) != m2->rows(m2) || m1->cols(m1) != m2->cols(m2)) {
        return 0;
    }
    for (int i = 0; i < m1->rows(m1); i++) {
        for (int j = 0; j < m1->cols(m1); j++) {
            if (fabs(m1->get(m1, i, j) - m2->get(m2, i, j)) > FLOAT_TOL) {
                return 0;
            }
        }
    }
    return 1;
}

static int prop_matrix_multiply_associative(int iteration) {
    (void)iteration;
    /* Use small dimensions to keep runtime reasonable. */
    int m = pbt_rand_int(1, 5);
    int n = pbt_rand_int(1, 5);
    int p = pbt_rand_int(1, 5);
    int q = pbt_rand_int(1, 5);

    matrix *A = create_matrix(m, n);
    matrix *B = create_matrix(n, p);
    matrix *C = create_matrix(p, q);
    if (!A || !B || !C) {
        if (A) { A->free(A); } if (B) { B->free(B); } if (C) { C->free(C); }
        return 0;
    }

    /* Fill with small random values to avoid large floating-point drift. */
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            A->set(A, i, j, pbt_rand_double(-5.0, 5.0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++)
            B->set(B, i, j, pbt_rand_double(-5.0, 5.0));
    for (int i = 0; i < p; i++)
        for (int j = 0; j < q; j++)
            C->set(C, i, j, pbt_rand_double(-5.0, 5.0));

    /* (A*B)*C */
    matrix *AB  = A->multiply(A, B);
    matrix *AB_C = AB ? AB->multiply(AB, C) : NULL;

    /* A*(B*C) */
    matrix *BC  = B->multiply(B, C);
    matrix *A_BC = BC ? A->multiply(A, BC) : NULL;

    int ok = 0;
    if (AB_C && A_BC) {
        ok = matrices_approx_equal(AB_C, A_BC);
    }

    if (AB)   { AB->free(AB); }
    if (AB_C) { AB_C->free(AB_C); }
    if (BC)   { BC->free(BC); }
    if (A_BC) { A_BC->free(A_BC); }
    A->free(A);
    B->free(B);
    C->free(C);
    return ok;
}

/* ------------------------------------------------------------------ */
/*  Main                                                              */
/* ------------------------------------------------------------------ */

int main(void) {
    pbt_seed();

    pbt_check("List: insert then get returns same element",
              PBT_DEFAULT_ITERATIONS, prop_list_insert_get);

    pbt_check("Sorting: result is non-decreasing sequence",
              PBT_DEFAULT_ITERATIONS, prop_sort_non_decreasing);

    pbt_check("Set: union is commutative",
              PBT_DEFAULT_ITERATIONS, prop_set_union_commutative);

    pbt_check("Set: union is associative",
              PBT_DEFAULT_ITERATIONS, prop_set_union_associative);

    pbt_check("Matrix: multiplication is associative",
              PBT_DEFAULT_ITERATIONS, prop_matrix_multiply_associative);

    return 0;
}
