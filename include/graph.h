#ifndef GRAPH_H
#define GRAPH_H

#include "list.h"
#include "matrix.h"
#include "types.h"

typedef enum {
    GRAPH_DIRECTED,
    GRAPH_UNDIRECTED
} graph_type;

typedef enum {
    GRAPH_ADJACENCY_LIST,
    GRAPH_ADJACENCY_MATRIX
} graph_representation;

typedef enum {
    GRAPH_TRAVERSE_BFS,
    GRAPH_TRAVERSE_DFS_ITERATIVE,
    GRAPH_TRAVERSE_DFS_RECURSIVE
} graph_traversal_strategy;

typedef enum {
    GRAPH_MIN_PATH_DIJKSTRA,
    GRAPH_MIN_PATH_BELLMAN_FORD
} graph_min_path_strategy;

/**
 * @brief Public weighted-edge result object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct graph_weighted_edge {
    void *impl;

    /**
     * @brief Return the source vertex id.
     *
     * @param self Edge instance.
     * @return Source vertex id.
     */
    int (*from)(const struct graph_weighted_edge *self);

    /**
     * @brief Return the destination vertex id.
     *
     * @param self Edge instance.
     * @return Destination vertex id.
     */
    int (*to)(const struct graph_weighted_edge *self);

    /**
     * @brief Return the edge weight.
     *
     * @param self Edge instance.
     * @return Edge weight.
     */
    double (*weight)(const struct graph_weighted_edge *self);

    /**
     * @brief Release the edge object.
     *
     * @param self Edge instance.
     */
    void (*free)(struct graph_weighted_edge *self);
} graph_weighted_edge;

/**
 * @brief Public graph object.
 *
 * Concrete storage details live in @p impl.
 */
typedef struct graph {
    void *impl;

    void (*add_vertex)(struct graph *self, int vertex_id);
    void (*add_edge)(struct graph *self, int from, int to, double weight);
    void (*remove_edge)(struct graph *self, int from, int to);
    bool (*has_edge)(const struct graph *self, int from, int to);
    list *(*get_neighbors)(const struct graph *self, int vertex);
    int (*vertex_count)(const struct graph *self);
    bool (*has_vertex)(const struct graph *self, int vertex_id);
    double (*get_edge_weight)(const struct graph *self, int from, int to);
    list *(*traverse)(const struct graph *self, int start_vertex);
    int (*min_path)(const struct graph *self, int start_vertex, vector **out_dist);
    list *(*mst_prim)(const struct graph *self, int start_vertex);
    list *(*mst_kruskal)(const struct graph *self);
    list *(*connected_components)(const struct graph *self);
    list *(*strongly_connected_components)(const struct graph *self);
    list *(*topological_sort)(const struct graph *self);
    bool (*has_cycle)(const struct graph *self);
    void (*free)(struct graph *self);
} graph;

graph *create_graph(graph_type type,
                    graph_representation rep,
                    graph_traversal_strategy traversal_strategy,
                    graph_min_path_strategy min_path_strategy);

#endif // GRAPH_H
