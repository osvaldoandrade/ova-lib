# Graphs

The graph module supports directed and undirected graphs with 2 storage backends: adjacency lists and adjacency matrices. The choice is made at construction time through `create_graph(graph_type type, graph_representation rep)`.

## Graph Model

Vertices are identified by non-negative `int` ids. The graph stores one heap-allocated `int *` per present vertex and reuses those pointers in traversal results. `graph_add_vertex` adds a single vertex. `graph_add_edge` also adds missing endpoint vertices implicitly, which is why many tests start from edges rather than explicit vertex creation.

Edge weights are `double`. A missing edge is represented internally by the `GRAPH_NO_EDGE` sentinel.

## Representations

`GRAPH_ADJACENCY_LIST` stores one list of edge records per present vertex. This keeps memory close to the number of edges and works well for BFS, DFS, Dijkstra, Prim, Kruskal, connected components, and strongly connected components on sparse graphs.

`GRAPH_ADJACENCY_MATRIX` stores one dense `vertex_capacity × vertex_capacity` matrix of weights. This makes edge lookup simple and keeps Floyd-Warshall direct, but storage grows with the square of the active capacity.

## Core API

The base mutation and inspection layer is:

```c
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
```

`graph_get_neighbors` returns an allocated list container even when the vertex is missing. The list elements are graph-owned `int *` vertex ids, so callers free the list container and keep their hands off the integers inside it.

## Algorithms

The traversal layer returns lists of graph-owned `int *` vertex ids:

- `graph_bfs`
- `graph_dfs_iterative`
- `graph_dfs_recursive`
- `graph_topological_sort`
- `graph_connected_components`
- `graph_strongly_connected_components`

For `graph_connected_components` and `graph_strongly_connected_components`, the outer list owns inner list containers, and each inner list holds graph-owned vertex pointers.

The shortest-path layer returns `1` on success and `0` on failure:

- `graph_dijkstra`
- `graph_bellman_ford`

On success, these functions allocate a `vector` and write it through `out_dist`. The caller must destroy that vector with `vector->destroy`.

`graph_floyd_warshall` returns a newly allocated `matrix *` or `NULL`. The caller must destroy the matrix.

The minimum-spanning-tree layer returns a list of heap-allocated `graph_weighted_edge *` records:

- `graph_mst_prim`
- `graph_mst_kruskal`

The caller must free each returned edge and then free the list container.

## Practical Rule

Choose adjacency lists unless you know edge density is high enough that matrix storage is acceptable and direct matrix-style scans are more useful than sparse traversal cost.
