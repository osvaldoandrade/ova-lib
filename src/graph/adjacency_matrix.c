#include "graph_internal.h"

static inline double *cell(const graph *g, int from, int to) {
    /* Defensive bounds checking to prevent out-of-bounds access.
     * Note: Public API functions validate vertex IDs before calling this,
     * but we keep these checks as a security safeguard against potential
     * future bugs or direct internal calls. The performance impact is minimal
     * due to inlining and branch prediction. */
    if (!g || !g->adj_matrix) {
        return NULL;
    }
    if (from < 0 || from >= g->vertex_capacity) {
        return NULL;
    }
    if (to < 0 || to >= g->vertex_capacity) {
        return NULL;
    }
    
    /* Calculate index using size_t to prevent overflow.
     * Since g->vertex_capacity is bounded by GRAPH_MAX_CAPACITY (46340),
     * and from/to are validated to be < vertex_capacity,
     * the multiplication is safe. */
    size_t index = (size_t)from * (size_t)g->vertex_capacity + (size_t)to;
    return &g->adj_matrix[index];
}

void graph_adj_matrix_add_edge(graph *g, int from, int to, double weight) {
    if (!g || !g->adj_matrix) {
        return;
    }

    *cell(g, from, to) = weight;
    if (g->type == GRAPH_UNDIRECTED && from != to) {
        *cell(g, to, from) = weight;
    }
}

void graph_adj_matrix_remove_edge(graph *g, int from, int to) {
    if (!g || !g->adj_matrix) {
        return;
    }

    *cell(g, from, to) = GRAPH_NO_EDGE;
    if (g->type == GRAPH_UNDIRECTED && from != to) {
        *cell(g, to, from) = GRAPH_NO_EDGE;
    }
}

bool graph_adj_matrix_has_edge(const graph *g, int from, int to) {
    if (!g || !g->adj_matrix) {
        return false;
    }
    return *cell(g, from, to) != GRAPH_NO_EDGE;
}

double graph_adj_matrix_get_edge_weight(const graph *g, int from, int to) {
    if (!g || !g->adj_matrix) {
        return GRAPH_NO_EDGE;
    }
    return *cell(g, from, to);
}

list *graph_adj_matrix_get_neighbors(const graph *g, int vertex) {
    list *neighbors = create_list(ARRAY_LIST, 4, NULL);
    if (!neighbors) {
        return NULL;
    }

    if (!g || !g->adj_matrix || vertex < 0 || vertex >= g->vertex_capacity) {
        return neighbors;
    }

    for (int to = 0; to < g->vertex_capacity; to++) {
        if (!g->present[to]) {
            continue;
        }
        if (*cell(g, vertex, to) == GRAPH_NO_EDGE) {
            continue;
        }
        neighbors->insert(neighbors, g->vertex_ptrs[to], neighbors->size(neighbors));
    }

    return neighbors;
}

