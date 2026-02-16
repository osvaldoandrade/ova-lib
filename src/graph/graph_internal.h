#ifndef GRAPH_INTERNAL_H
#define GRAPH_INTERNAL_H

#include "../../include/graph.h"
#include <math.h>

#define GRAPH_NO_EDGE (INFINITY)

typedef struct graph_edge {
    int to;
    double weight;
} graph_edge;

struct graph {
    graph_type type;
    graph_representation rep;

    int vertex_capacity;
    int vertex_count;
    bool *present;
    int **vertex_ptrs;

    list **adj_lists;    /* adjacency list: list of graph_edge* per vertex */
    double *adj_matrix;  /* adjacency matrix: row-major weights (GRAPH_NO_EDGE = no edge) */
};

void graph_ensure_capacity(graph *g, int vertex_id);

/* Adjacency list backend */
void graph_adj_list_add_edge(graph *g, int from, int to, double weight);
void graph_adj_list_remove_edge(graph *g, int from, int to);
bool graph_adj_list_has_edge(const graph *g, int from, int to);
double graph_adj_list_get_edge_weight(const graph *g, int from, int to);
list *graph_adj_list_get_neighbors(const graph *g, int vertex);
void graph_adj_list_free_edges(graph *g);

/* Adjacency matrix backend */
void graph_adj_matrix_add_edge(graph *g, int from, int to, double weight);
void graph_adj_matrix_remove_edge(graph *g, int from, int to);
bool graph_adj_matrix_has_edge(const graph *g, int from, int to);
double graph_adj_matrix_get_edge_weight(const graph *g, int from, int to);
list *graph_adj_matrix_get_neighbors(const graph *g, int vertex);

#endif // GRAPH_INTERNAL_H
