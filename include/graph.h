#ifndef GRAPH_H
#define GRAPH_H

/**
 * @file graph.h
 * @brief Graph data structure with adjacency list and adjacency matrix representations.
 *
 * Provides directed and undirected graph operations including traversals (BFS, DFS),
 * shortest-path algorithms (Dijkstra, Bellman-Ford, Floyd-Warshall), minimum spanning
 * tree algorithms (Prim, Kruskal), connectivity analysis, topological sorting, and
 * cycle detection.
 *
 * Example Usage:
 * @code
 *   graph *g = create_graph(GRAPH_UNDIRECTED, GRAPH_ADJACENCY_LIST);
 *   graph_add_vertex(g, 0);
 *   graph_add_vertex(g, 1);
 *   graph_add_edge(g, 0, 1, 1.0);
 *   list *neighbors = graph_get_neighbors(g, 0);
 *   graph_free(g);
 * @endcode
 */

#include "list.h"
#include "types.h"

typedef struct vector vector;
typedef struct matrix matrix;

/**
 * @brief Specifies whether a graph is directed or undirected.
 */
typedef enum {
    GRAPH_DIRECTED,     /**< Directed graph (edges have a direction). */
    GRAPH_UNDIRECTED    /**< Undirected graph (edges are bidirectional). */
} graph_type;

/**
 * @brief Specifies the internal representation of a graph.
 */
typedef enum {
    GRAPH_ADJACENCY_LIST,   /**< Adjacency list representation. */
    GRAPH_ADJACENCY_MATRIX  /**< Adjacency matrix representation. */
} graph_representation;

typedef struct graph graph;

/**
 * @brief Represents a weighted edge in a graph.
 */
typedef struct graph_weighted_edge {
    int from;       /**< Source vertex identifier. */
    int to;         /**< Destination vertex identifier. */
    double weight;  /**< Weight of the edge. */
} graph_weighted_edge;

/**
 * @brief Creates a new graph.
 *
 * @param type Whether the graph is directed or undirected.
 * @param rep Internal representation (adjacency list or adjacency matrix).
 * @return A pointer to the newly created graph, or NULL on allocation failure.
 */
graph *create_graph(graph_type type, graph_representation rep);

/**
 * @brief Frees all memory associated with the graph.
 *
 * @param g The graph to free.
 */
void graph_free(graph *g);

/**
 * @brief Adds a vertex to the graph.
 *
 * @param g The graph.
 * @param vertex_id Unique identifier for the new vertex.
 */
void graph_add_vertex(graph *g, int vertex_id);

/**
 * @brief Adds a weighted edge to the graph.
 *
 * For undirected graphs the edge is added in both directions.
 *
 * @param g The graph.
 * @param from Source vertex identifier.
 * @param to Destination vertex identifier.
 * @param weight Weight of the edge.
 */
void graph_add_edge(graph *g, int from, int to, double weight);

/**
 * @brief Removes an edge from the graph.
 *
 * @param g The graph.
 * @param from Source vertex identifier.
 * @param to Destination vertex identifier.
 */
void graph_remove_edge(graph *g, int from, int to);

/**
 * @brief Checks whether an edge exists between two vertices.
 *
 * @param g The graph.
 * @param from Source vertex identifier.
 * @param to Destination vertex identifier.
 * @return true if the edge exists, false otherwise.
 */
bool graph_has_edge(const graph *g, int from, int to);

/**
 * @brief Returns the neighbors of a vertex.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param g The graph.
 * @param vertex The vertex whose neighbors are requested.
 * @return A list of neighbor vertex identifiers.
 */
list *graph_get_neighbors(const graph *g, int vertex);

/**
 * @brief Returns the number of vertices in the graph.
 *
 * @param g The graph.
 * @return The vertex count.
 */
int graph_vertex_count(const graph *g);

/**
 * @brief Checks whether a vertex exists in the graph.
 *
 * @param g The graph.
 * @param vertex_id The vertex identifier to check.
 * @return true if the vertex exists, false otherwise.
 */
bool graph_has_vertex(const graph *g, int vertex_id);

/**
 * @brief Returns the weight of the edge between two vertices.
 *
 * @param g The graph.
 * @param from Source vertex identifier.
 * @param to Destination vertex identifier.
 * @return The edge weight, or INFINITY if the edge does not exist.
 */
double graph_get_edge_weight(const graph *g, int from, int to);

/**
 * @brief Performs a breadth-first search starting from the given vertex.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param g The graph.
 * @param start_vertex The vertex to start the search from.
 * @return A list of vertex identifiers in BFS order.
 */
list *graph_bfs(const graph *g, int start_vertex);

/**
 * @brief Performs an iterative depth-first search starting from the given vertex.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param g The graph.
 * @param start_vertex The vertex to start the search from.
 * @return A list of vertex identifiers in DFS order.
 */
list *graph_dfs_iterative(const graph *g, int start_vertex);

/**
 * @brief Performs a recursive depth-first search starting from the given vertex.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param g The graph.
 * @param start_vertex The vertex to start the search from.
 * @return A list of vertex identifiers in DFS order.
 */
list *graph_dfs_recursive(const graph *g, int start_vertex);

/**
 * @brief Computes shortest paths from a source vertex using Dijkstra's algorithm.
 *
 * Requires non-negative edge weights.
 *
 * @param g The graph.
 * @param start_vertex The source vertex.
 * @param[out] out_dist Pointer to a vector that will receive the shortest distances.
 * @return 0 on success, or a negative value on error.
 *
 * @see graph_bellman_ford
 */
int graph_dijkstra(const graph *g, int start_vertex, vector **out_dist);

/**
 * @brief Computes shortest paths from a source vertex using the Bellman-Ford algorithm.
 *
 * Handles negative edge weights and detects negative-weight cycles.
 *
 * @param g The graph.
 * @param start_vertex The source vertex.
 * @param[out] out_dist Pointer to a vector that will receive the shortest distances.
 * @return 0 on success, or a negative value on error or negative-weight cycle.
 *
 * @see graph_dijkstra
 */
int graph_bellman_ford(const graph *g, int start_vertex, vector **out_dist);

/**
 * @brief Computes all-pairs shortest paths using the Floyd-Warshall algorithm.
 *
 * The caller owns the returned matrix and must free it with matrix->destroy(matrix).
 *
 * @param g The graph.
 * @return A distance matrix, or NULL on error.
 *
 * @see graph_dijkstra, graph_bellman_ford
 */
matrix *graph_floyd_warshall(const graph *g);

/**
 * @brief Computes a minimum spanning tree using Prim's algorithm.
 *
 * The caller owns the returned list of graph_weighted_edge and must free it.
 *
 * @param g The graph (must be undirected and connected from @p start_vertex).
 * @param start_vertex The vertex to start from.
 * @return A list of graph_weighted_edge forming the MST.
 *
 * @see graph_mst_kruskal
 */
list *graph_mst_prim(const graph *g, int start_vertex);

/**
 * @brief Computes a minimum spanning tree using Kruskal's algorithm.
 *
 * The caller owns the returned list of graph_weighted_edge and must free it.
 *
 * @param g The graph (must be undirected).
 * @return A list of graph_weighted_edge forming the MST.
 *
 * @see graph_mst_prim
 */
list *graph_mst_kruskal(const graph *g);

/**
 * @brief Finds connected components of an undirected graph.
 *
 * Each component is represented as a list of vertex identifiers.
 * The caller owns the returned list of lists and must free them.
 *
 * @param g The graph.
 * @return A list of lists, each containing vertex identifiers of one component.
 */
list *graph_connected_components(const graph *g);

/**
 * @brief Finds strongly connected components of a directed graph.
 *
 * Each component is represented as a list of vertex identifiers.
 * The caller owns the returned list of lists and must free them.
 *
 * @param g The graph.
 * @return A list of lists, each containing vertex identifiers of one SCC.
 */
list *graph_strongly_connected_components(const graph *g);

/**
 * @brief Produces a topological ordering of a directed acyclic graph.
 *
 * The caller owns the returned list and must free it with list->free(list).
 *
 * @param g The graph (must be a DAG).
 * @return A list of vertex identifiers in topological order, or NULL if the graph has a cycle.
 *
 * @see graph_has_cycle
 */
list *graph_topological_sort(const graph *g);

/**
 * @brief Checks whether the graph contains a cycle.
 *
 * @param g The graph.
 * @return true if the graph has at least one cycle, false otherwise.
 *
 * @see graph_topological_sort
 */
bool graph_has_cycle(const graph *g);

#endif // GRAPH_H
