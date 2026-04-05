#ifndef GRAPH_H
#define GRAPH_H

#include "list.h"
#include "matrix.h"
#include "types.h"

typedef enum {
    GRAPH_DIRECTED,
    GRAPH_UNDIRECTED
} graph_type;

/**
 * @brief Select the storage model for graph edges.
 */
typedef enum {
    GRAPH_ADJACENCY_LIST,
    GRAPH_ADJACENCY_MATRIX
} graph_representation;

/**
 * @brief Select the traversal algorithm bound to @c graph->traverse.
 */
typedef enum {
    GRAPH_TRAVERSE_BFS,
    GRAPH_TRAVERSE_DFS_ITERATIVE,
    GRAPH_TRAVERSE_DFS_RECURSIVE
} graph_traversal_strategy;

/**
 * @brief Select the shortest-path algorithm bound to @c graph->min_path.
 */
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

    /**
     * @brief Traverse from a start vertex using the strategy chosen at construction time.
     *
     * Returns an empty list when the start vertex is invalid.
     */
    list *(*traverse)(const struct graph *self, int start_vertex);

    /**
     * @brief Compute single-source shortest paths using the strategy chosen at construction time.
     *
     * Returns 1 on success, 0 on failure. The returned vector belongs to the caller.
     */
    int (*min_path)(const struct graph *self, int start_vertex, vector **out_dist);
    list *(*mst_prim)(const struct graph *self, int start_vertex);
    list *(*mst_kruskal)(const struct graph *self);
    list *(*connected_components)(const struct graph *self);
    list *(*strongly_connected_components)(const struct graph *self);
    list *(*topological_sort)(const struct graph *self);
    bool (*has_cycle)(const struct graph *self);
    void (*free)(struct graph *self);
} graph;

/**
 * @brief Create a graph with fixed structure and algorithm strategies.
 *
 * @param type Directed or undirected semantics.
 * @param rep Adjacency storage representation.
 * @param traversal_strategy Algorithm exposed through @c traverse.
 * @param min_path_strategy Algorithm exposed through @c min_path.
 * @return New graph instance, or NULL on invalid configuration or allocation failure.
 */
graph *create_graph(graph_type type,
                    graph_representation rep,
                    graph_traversal_strategy traversal_strategy,
                    graph_min_path_strategy min_path_strategy);

#endif // GRAPH_H
