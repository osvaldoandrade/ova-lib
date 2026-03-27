#ifndef GRAPH_H
#define GRAPH_H

#include "list.h"
#include "types.h"

typedef struct vector vector;
typedef struct matrix matrix;

typedef enum {
    GRAPH_DIRECTED,
    GRAPH_UNDIRECTED
} graph_type;

typedef enum {
    GRAPH_ADJACENCY_LIST,
    GRAPH_ADJACENCY_MATRIX
} graph_representation;

typedef struct graph graph;

typedef struct graph_weighted_edge {
    int from;
    int to;
    double weight;
} graph_weighted_edge;

graph *create_graph(graph_type type, graph_representation rep);
void graph_free(graph *g);

void graph_add_vertex(graph *g, int vertex_id);
void graph_add_edge(graph *g, int from, int to, double weight);
void graph_remove_edge(graph *g, int from, int to);
bool graph_has_edge(const graph *g, int from, int to);
list *graph_get_neighbors(const graph *g, int vertex);

int graph_vertex_count(const graph *g);
bool graph_has_vertex(const graph *g, int vertex_id);
double graph_get_edge_weight(const graph *g, int from, int to);

list *graph_bfs(const graph *g, int start_vertex);
list *graph_dfs_iterative(const graph *g, int start_vertex);
list *graph_dfs_recursive(const graph *g, int start_vertex);

int graph_dijkstra(const graph *g, int start_vertex, vector **out_dist);
int graph_bellman_ford(const graph *g, int start_vertex, vector **out_dist);
matrix *graph_floyd_warshall(const graph *g);

list *graph_mst_prim(const graph *g, int start_vertex);
list *graph_mst_kruskal(const graph *g);

list *graph_connected_components(const graph *g);
list *graph_strongly_connected_components(const graph *g);

list *graph_topological_sort(const graph *g);
bool graph_has_cycle(const graph *g);

#endif // GRAPH_H
