#include "../../include/graph.h"
#include "graph_internal.h"

#include <stdlib.h>
#include <string.h>

static int graph_weighted_edge_from_method(const graph_weighted_edge *self) {
    graph_weighted_edge_impl *impl = graph_weighted_edge_impl_from_public(self);
    return impl ? impl->from : -1;
}

static int graph_weighted_edge_to_method(const graph_weighted_edge *self) {
    graph_weighted_edge_impl *impl = graph_weighted_edge_impl_from_public(self);
    return impl ? impl->to : -1;
}

static double graph_weighted_edge_weight_method(const graph_weighted_edge *self) {
    graph_weighted_edge_impl *impl = graph_weighted_edge_impl_from_public(self);
    return impl ? impl->weight : 0.0;
}

static void graph_weighted_edge_free_method(graph_weighted_edge *self) {
    if (!self) {
        return;
    }
    free(self->impl);
    self->impl = NULL;
    free(self);
}

graph_weighted_edge *graph_create_weighted_edge(int from, int to, double weight) {
    graph_weighted_edge *out = (graph_weighted_edge *)calloc(1, sizeof(graph_weighted_edge));
    if (!out) {
        return NULL;
    }

    graph_weighted_edge_impl *impl = (graph_weighted_edge_impl *)calloc(1, sizeof(graph_weighted_edge_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->from = from;
    impl->to = to;
    impl->weight = weight;

    out->impl = impl;
    out->from = graph_weighted_edge_from_method;
    out->to = graph_weighted_edge_to_method;
    out->weight = graph_weighted_edge_weight_method;
    out->free = graph_weighted_edge_free_method;
    return out;
}

static int clamp_initial_capacity(int requested) {
    return requested > 0 ? requested : 8;
}

static int is_valid_vertex_id(int vertex_id) {
    return vertex_id >= 0;
}

static int graph_init_storage(graph_impl *g, int initial_capacity) {
    int cap = clamp_initial_capacity(initial_capacity);

    g->present = (bool *)calloc((size_t)cap, sizeof(bool));
    g->vertex_ptrs = (int **)calloc((size_t)cap, sizeof(int *));
    if (!g->present || !g->vertex_ptrs) {
        free(g->present);
        free(g->vertex_ptrs);
        g->present = NULL;
        g->vertex_ptrs = NULL;
        return 0;
    }

    g->adj_lists = NULL;
    g->adj_matrix = NULL;

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        g->adj_lists = (list **)calloc((size_t)cap, sizeof(list *));
        if (!g->adj_lists) {
            free(g->present);
            free(g->vertex_ptrs);
            g->present = NULL;
            g->vertex_ptrs = NULL;
            return 0;
        }
    } else if (g->rep == GRAPH_ADJACENCY_MATRIX) {
        g->adj_matrix = (double *)malloc((size_t)cap * (size_t)cap * sizeof(double));
        if (!g->adj_matrix) {
            free(g->present);
            free(g->vertex_ptrs);
            g->present = NULL;
            g->vertex_ptrs = NULL;
            return 0;
        }
        for (int i = 0; i < cap * cap; i++) {
            g->adj_matrix[i] = GRAPH_NO_EDGE;
        }
    }

    g->vertex_capacity = cap;
    return 1;
}

void graph_ensure_capacity(graph_impl *g, int vertex_id) {
    if (!g || !is_valid_vertex_id(vertex_id)) {
        return;
    }

    int required = vertex_id + 1;
    if (required <= g->vertex_capacity) {
        return;
    }

    int old_cap = g->vertex_capacity;
    int new_cap = old_cap > 0 ? old_cap : 8;
    while (new_cap < required) {
        new_cap *= 2;
    }

    bool *new_present = (bool *)calloc((size_t)new_cap, sizeof(bool));
    int **new_vertex_ptrs = (int **)calloc((size_t)new_cap, sizeof(int *));
    list **new_adj_lists = NULL;
    double *new_adj_matrix = NULL;

    if (!new_present || !new_vertex_ptrs) {
        free(new_present);
        free(new_vertex_ptrs);
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        new_adj_lists = (list **)calloc((size_t)new_cap, sizeof(list *));
        if (!new_adj_lists) {
            free(new_present);
            free(new_vertex_ptrs);
            return;
        }
    } else if (g->rep == GRAPH_ADJACENCY_MATRIX) {
        new_adj_matrix = (double *)malloc((size_t)new_cap * (size_t)new_cap * sizeof(double));
        if (!new_adj_matrix) {
            free(new_present);
            free(new_vertex_ptrs);
            return;
        }
        for (int i = 0; i < new_cap * new_cap; i++) {
            new_adj_matrix[i] = GRAPH_NO_EDGE;
        }
    }

    if (g->present) {
        memcpy(new_present, g->present, (size_t)old_cap * sizeof(bool));
    }
    if (g->vertex_ptrs) {
        memcpy(new_vertex_ptrs, g->vertex_ptrs, (size_t)old_cap * sizeof(int *));
    }
    if (g->rep == GRAPH_ADJACENCY_LIST && g->adj_lists) {
        memcpy(new_adj_lists, g->adj_lists, (size_t)old_cap * sizeof(list *));
    }
    if (g->rep == GRAPH_ADJACENCY_MATRIX && g->adj_matrix) {
        for (int i = 0; i < old_cap; i++) {
            memcpy(&new_adj_matrix[i * new_cap], &g->adj_matrix[i * old_cap], (size_t)old_cap * sizeof(double));
        }
    }

    free(g->present);
    free(g->vertex_ptrs);
    free(g->adj_lists);
    free(g->adj_matrix);

    g->present = new_present;
    g->vertex_ptrs = new_vertex_ptrs;
    g->adj_lists = new_adj_lists;
    g->adj_matrix = new_adj_matrix;
    g->vertex_capacity = new_cap;
}

bool graph_has_vertex_impl(const graph_impl *g, int vertex_id) {
    if (!g || !is_valid_vertex_id(vertex_id) || vertex_id >= g->vertex_capacity) {
        return false;
    }
    return g->present[vertex_id];
}

int graph_vertex_count_impl(const graph_impl *g) {
    return g ? g->vertex_count : 0;
}

void graph_add_vertex_impl(graph_impl *g, int vertex_id) {
    if (!g || !is_valid_vertex_id(vertex_id)) {
        return;
    }

    graph_ensure_capacity(g, vertex_id);
    if (vertex_id >= g->vertex_capacity || g->present[vertex_id]) {
        return;
    }

    int *id_ptr = (int *)malloc(sizeof(int));
    if (!id_ptr) {
        return;
    }
    *id_ptr = vertex_id;

    g->vertex_ptrs[vertex_id] = id_ptr;
    g->present[vertex_id] = true;
    g->vertex_count++;

    if (g->rep == GRAPH_ADJACENCY_LIST && g->adj_lists && !g->adj_lists[vertex_id]) {
        g->adj_lists[vertex_id] = create_list(ARRAY_LIST, 4, NULL);
    }
}

void graph_add_edge_impl(graph_impl *g, int from, int to, double weight) {
    if (!g || !is_valid_vertex_id(from) || !is_valid_vertex_id(to)) {
        return;
    }

    graph_add_vertex_impl(g, from);
    graph_add_vertex_impl(g, to);
    if (!graph_has_vertex_impl(g, from) || !graph_has_vertex_impl(g, to)) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        graph_adj_list_add_edge(g, from, to, weight);
    } else {
        graph_adj_matrix_add_edge(g, from, to, weight);
    }
}

void graph_remove_edge_impl(graph_impl *g, int from, int to) {
    if (!g || !graph_has_vertex_impl(g, from) || !graph_has_vertex_impl(g, to)) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        graph_adj_list_remove_edge(g, from, to);
    } else {
        graph_adj_matrix_remove_edge(g, from, to);
    }
}

bool graph_has_edge_impl(const graph_impl *g, int from, int to) {
    if (!g || !graph_has_vertex_impl(g, from) || !graph_has_vertex_impl(g, to)) {
        return false;
    }

    return (g->rep == GRAPH_ADJACENCY_LIST)
        ? graph_adj_list_has_edge(g, from, to)
        : graph_adj_matrix_has_edge(g, from, to);
}

double graph_get_edge_weight_impl(const graph_impl *g, int from, int to) {
    if (!g || !graph_has_vertex_impl(g, from) || !graph_has_vertex_impl(g, to)) {
        return GRAPH_NO_EDGE;
    }

    return (g->rep == GRAPH_ADJACENCY_LIST)
        ? graph_adj_list_get_edge_weight(g, from, to)
        : graph_adj_matrix_get_edge_weight(g, from, to);
}

list *graph_get_neighbors_impl(const graph_impl *g, int vertex) {
    if (!g || !graph_has_vertex_impl(g, vertex)) {
        return create_list(ARRAY_LIST, 4, NULL);
    }

    return (g->rep == GRAPH_ADJACENCY_LIST)
        ? graph_adj_list_get_neighbors(g, vertex)
        : graph_adj_matrix_get_neighbors(g, vertex);
}

static void graph_add_vertex_method(graph *self, int vertex_id) {
    graph_add_vertex_impl(graph_impl_from_graph(self), vertex_id);
}

static void graph_add_edge_method(graph *self, int from, int to, double weight) {
    graph_add_edge_impl(graph_impl_from_graph(self), from, to, weight);
}

static void graph_remove_edge_method(graph *self, int from, int to) {
    graph_remove_edge_impl(graph_impl_from_graph(self), from, to);
}

static bool graph_has_edge_method(const graph *self, int from, int to) {
    return graph_has_edge_impl(graph_impl_from_graph(self), from, to);
}

static list *graph_get_neighbors_method(const graph *self, int vertex) {
    return graph_get_neighbors_impl(graph_impl_from_graph(self), vertex);
}

static int graph_vertex_count_method(const graph *self) {
    return graph_vertex_count_impl(graph_impl_from_graph(self));
}

static bool graph_has_vertex_method(const graph *self, int vertex_id) {
    return graph_has_vertex_impl(graph_impl_from_graph(self), vertex_id);
}

static double graph_get_edge_weight_method(const graph *self, int from, int to) {
    return graph_get_edge_weight_impl(graph_impl_from_graph(self), from, to);
}

static list *graph_bfs_method(const graph *self, int start_vertex) {
    return graph_bfs_impl(graph_impl_from_graph(self), start_vertex);
}

static list *graph_dfs_iterative_method(const graph *self, int start_vertex) {
    return graph_dfs_iterative_impl(graph_impl_from_graph(self), start_vertex);
}

static list *graph_dfs_recursive_method(const graph *self, int start_vertex) {
    return graph_dfs_recursive_impl(graph_impl_from_graph(self), start_vertex);
}

static int graph_dijkstra_method(const graph *self, int start_vertex, vector **out_dist) {
    return graph_dijkstra_impl(graph_impl_from_graph(self), start_vertex, out_dist);
}

static int graph_bellman_ford_method(const graph *self, int start_vertex, vector **out_dist) {
    return graph_bellman_ford_impl(graph_impl_from_graph(self), start_vertex, out_dist);
}

static matrix *graph_floyd_warshall_method(const graph *self) {
    return graph_floyd_warshall_impl(graph_impl_from_graph(self));
}

static list *graph_mst_prim_method(const graph *self, int start_vertex) {
    return graph_mst_prim_impl(graph_impl_from_graph(self), start_vertex);
}

static list *graph_mst_kruskal_method(const graph *self) {
    return graph_mst_kruskal_impl(graph_impl_from_graph(self));
}

static list *graph_connected_components_method(const graph *self) {
    return graph_connected_components_impl(graph_impl_from_graph(self));
}

static list *graph_strongly_connected_components_method(const graph *self) {
    return graph_strongly_connected_components_impl(graph_impl_from_graph(self));
}

static list *graph_topological_sort_method(const graph *self) {
    return graph_topological_sort_impl(graph_impl_from_graph(self));
}

static bool graph_has_cycle_method(const graph *self) {
    return graph_has_cycle_impl(graph_impl_from_graph(self));
}

static void graph_free_method(graph *self) {
    if (!self) {
        return;
    }

    graph_impl *impl = graph_impl_from_graph(self);
    if (impl) {
        if (impl->rep == GRAPH_ADJACENCY_LIST) {
            graph_adj_list_free_edges(impl);
        }

        for (int i = 0; i < impl->vertex_capacity; i++) {
            free(impl->vertex_ptrs ? impl->vertex_ptrs[i] : NULL);
        }

        free(impl->present);
        free(impl->vertex_ptrs);
        free(impl->adj_lists);
        free(impl->adj_matrix);
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

graph *create_graph(graph_type type, graph_representation rep) {
    graph *out = (graph *)calloc(1, sizeof(graph));
    if (!out) {
        return NULL;
    }

    graph_impl *impl = (graph_impl *)calloc(1, sizeof(graph_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->type = type;
    impl->rep = rep;
    impl->vertex_capacity = 0;
    impl->vertex_count = 0;
    impl->present = NULL;
    impl->vertex_ptrs = NULL;
    impl->adj_lists = NULL;
    impl->adj_matrix = NULL;

    if (!graph_init_storage(impl, 8)) {
        free(impl);
        free(out);
        return NULL;
    }

    out->impl = impl;
    out->add_vertex = graph_add_vertex_method;
    out->add_edge = graph_add_edge_method;
    out->remove_edge = graph_remove_edge_method;
    out->has_edge = graph_has_edge_method;
    out->get_neighbors = graph_get_neighbors_method;
    out->vertex_count = graph_vertex_count_method;
    out->has_vertex = graph_has_vertex_method;
    out->get_edge_weight = graph_get_edge_weight_method;
    out->bfs = graph_bfs_method;
    out->dfs_iterative = graph_dfs_iterative_method;
    out->dfs_recursive = graph_dfs_recursive_method;
    out->dijkstra = graph_dijkstra_method;
    out->bellman_ford = graph_bellman_ford_method;
    out->floyd_warshall = graph_floyd_warshall_method;
    out->mst_prim = graph_mst_prim_method;
    out->mst_kruskal = graph_mst_kruskal_method;
    out->connected_components = graph_connected_components_method;
    out->strongly_connected_components = graph_strongly_connected_components_method;
    out->topological_sort = graph_topological_sort_method;
    out->has_cycle = graph_has_cycle_method;
    out->free = graph_free_method;

    return out;
}
