#ifndef GRAPH_INTERNAL_H
#define GRAPH_INTERNAL_H

#include "../../include/graph.h"

#include <math.h>

#define GRAPH_NO_EDGE ((double)INFINITY)

typedef struct graph_edge {
    int to;
    double weight;
} graph_edge;

typedef struct graph_impl {
    graph_type type;
    graph_representation rep;
    int vertex_capacity;
    int vertex_count;
    bool *present;
    int **vertex_ptrs;
    list **adj_lists;
    double *adj_matrix;
} graph_impl;

typedef struct graph_weighted_edge_impl {
    int from;
    int to;
    double weight;
} graph_weighted_edge_impl;

static inline graph_impl *graph_impl_from_graph(const graph *g) {
    return g ? (graph_impl *)g->impl : NULL;
}

static inline graph_weighted_edge_impl *graph_weighted_edge_impl_from_public(const graph_weighted_edge *e) {
    return e ? (graph_weighted_edge_impl *)e->impl : NULL;
}

graph_weighted_edge *graph_create_weighted_edge(int from, int to, double weight);

void graph_ensure_capacity(graph_impl *g, int vertex_id);
void graph_add_vertex_impl(graph_impl *g, int vertex_id);
void graph_add_edge_impl(graph_impl *g, int from, int to, double weight);
void graph_remove_edge_impl(graph_impl *g, int from, int to);
bool graph_has_edge_impl(const graph_impl *g, int from, int to);
list *graph_get_neighbors_impl(const graph_impl *g, int vertex);
int graph_vertex_count_impl(const graph_impl *g);
bool graph_has_vertex_impl(const graph_impl *g, int vertex_id);
double graph_get_edge_weight_impl(const graph_impl *g, int from, int to);
void graph_adj_list_free_edges(graph_impl *g);

void graph_adj_list_add_edge(graph_impl *g, int from, int to, double weight);
void graph_adj_list_remove_edge(graph_impl *g, int from, int to);
bool graph_adj_list_has_edge(const graph_impl *g, int from, int to);
double graph_adj_list_get_edge_weight(const graph_impl *g, int from, int to);
list *graph_adj_list_get_neighbors(const graph_impl *g, int vertex);

void graph_adj_matrix_add_edge(graph_impl *g, int from, int to, double weight);
void graph_adj_matrix_remove_edge(graph_impl *g, int from, int to);
bool graph_adj_matrix_has_edge(const graph_impl *g, int from, int to);
double graph_adj_matrix_get_edge_weight(const graph_impl *g, int from, int to);
list *graph_adj_matrix_get_neighbors(const graph_impl *g, int vertex);

list *graph_bfs_impl(const graph_impl *g, int start_vertex);
list *graph_dfs_iterative_impl(const graph_impl *g, int start_vertex);
list *graph_dfs_recursive_impl(const graph_impl *g, int start_vertex);
int graph_dijkstra_impl(const graph_impl *g, int start_vertex, vector **out_dist);
int graph_bellman_ford_impl(const graph_impl *g, int start_vertex, vector **out_dist);
matrix *graph_floyd_warshall_impl(const graph_impl *g);
list *graph_mst_prim_impl(const graph_impl *g, int start_vertex);
list *graph_mst_kruskal_impl(const graph_impl *g);
list *graph_connected_components_impl(const graph_impl *g);
list *graph_strongly_connected_components_impl(const graph_impl *g);
list *graph_topological_sort_impl(const graph_impl *g);
bool graph_has_cycle_impl(const graph_impl *g);

#endif // GRAPH_INTERNAL_H
