# Graphs

ova-lib provides a lightweight graph abstraction with two storage backends:

- **Adjacency list**: efficient for sparse graphs.
- **Adjacency matrix**: simple representation, efficient for dense graphs and Floyd-Warshall.

Both **directed** and **undirected** graphs are supported.

## Data Structure API

```c
#include "graph.h"

graph *g = create_graph(GRAPH_UNDIRECTED, GRAPH_ADJACENCY_LIST);
graph_add_vertex(g, 0);
graph_add_vertex(g, 1);
graph_add_edge(g, 0, 1, 1.0);

if (graph_has_edge(g, 0, 1)) {
  double w = graph_get_edge_weight(g, 0, 1);
}

list *neighbors = graph_get_neighbors(g, 0);
neighbors->free(neighbors);

graph_free(g);
```

Notes:
- Vertex ids are non-negative integers.
- `graph_add_edge()` implicitly adds missing vertices.
- Containers returned by the graph APIs (`list*`) must be freed by the caller, but the payload pointers in those lists are owned by the graph.

## Algorithms API

Traversal:
- `graph_bfs()`
- `graph_dfs_iterative()`
- `graph_dfs_recursive()`

Shortest paths:
- `graph_dijkstra()` (non-negative weights; uses a binary heap without decrease-key)
- `graph_bellman_ford()` (returns 0 when a reachable negative cycle is detected)
- `graph_floyd_warshall()`

Minimum spanning tree (undirected graphs):
- `graph_mst_prim()`
- `graph_mst_kruskal()`

Connectivity:
- `graph_connected_components()` (undirected graphs)
- `graph_strongly_connected_components()` (directed graphs; Tarjan)

Other:
- `graph_topological_sort()` (directed graphs; returns `NULL` when the graph contains a cycle)
- `graph_has_cycle()`

### Memory Ownership

- `graph_bfs()/graph_dfs_*()/graph_get_neighbors()/graph_connected_components()/graph_strongly_connected_components()/graph_topological_sort()` return `list*` containers that must be freed by the caller. The vertex pointers inside are owned by the graph.
- `graph_dijkstra()/graph_bellman_ford()` write a `vector*` to `out_dist`; the caller must call `destroy()` on it.
- `graph_floyd_warshall()` returns a `matrix*`; the caller must call `destroy()` on it.
- MST functions return a `list*` of `graph_weighted_edge*`; the caller must free each edge and then free the list.

