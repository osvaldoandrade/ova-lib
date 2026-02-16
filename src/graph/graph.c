#include "../../include/graph.h"
#include "graph_internal.h"

#include <stdlib.h>
#include <string.h>

static int clamp_initial_capacity(int requested) {
    return requested > 0 ? requested : 8;
}

static int is_valid_vertex_id(int vertex_id) {
    return vertex_id >= 0;
}

static int graph_init_storage(graph *g, int initial_capacity) {
    int cap = clamp_initial_capacity(initial_capacity);

    g->present = calloc((size_t)cap, sizeof(bool));
    g->vertex_ptrs = calloc((size_t)cap, sizeof(int *));

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
        g->adj_lists = calloc((size_t)cap, sizeof(list *));
        if (!g->adj_lists) {
            free(g->present);
            free(g->vertex_ptrs);
            g->present = NULL;
            g->vertex_ptrs = NULL;
            return 0;
        }
    } else if (g->rep == GRAPH_ADJACENCY_MATRIX) {
        g->adj_matrix = malloc((size_t)cap * (size_t)cap * sizeof(double));
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

void graph_ensure_capacity(graph *g, int vertex_id) {
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

    bool *new_present = calloc((size_t)new_cap, sizeof(bool));
    int **new_vertex_ptrs = calloc((size_t)new_cap, sizeof(int *));
    list **new_adj_lists = NULL;
    double *new_adj_matrix = NULL;

    if (!new_present || !new_vertex_ptrs) {
        free(new_present);
        free(new_vertex_ptrs);
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        new_adj_lists = calloc((size_t)new_cap, sizeof(list *));
        if (!new_adj_lists) {
            free(new_present);
            free(new_vertex_ptrs);
            return;
        }
    } else if (g->rep == GRAPH_ADJACENCY_MATRIX) {
        new_adj_matrix = malloc((size_t)new_cap * (size_t)new_cap * sizeof(double));
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

graph *create_graph(graph_type type, graph_representation rep) {
    graph *g = calloc(1, sizeof(graph));
    if (!g) {
        return NULL;
    }

    g->type = type;
    g->rep = rep;
    g->vertex_capacity = 0;
    g->vertex_count = 0;
    g->present = NULL;
    g->vertex_ptrs = NULL;
    g->adj_lists = NULL;
    g->adj_matrix = NULL;

    if (!graph_init_storage(g, 8)) {
        free(g);
        return NULL;
    }

    return g;
}

void graph_free(graph *g) {
    if (!g) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        graph_adj_list_free_edges(g);
    }

    for (int i = 0; i < g->vertex_capacity; i++) {
        free(g->vertex_ptrs ? g->vertex_ptrs[i] : NULL);
    }

    free(g->present);
    free(g->vertex_ptrs);
    free(g->adj_lists);
    free(g->adj_matrix);
    free(g);
}

bool graph_has_vertex(const graph *g, int vertex_id) {
    if (!g || !is_valid_vertex_id(vertex_id) || vertex_id >= g->vertex_capacity) {
        return false;
    }
    return g->present[vertex_id];
}

int graph_vertex_count(const graph *g) {
    return g ? g->vertex_count : 0;
}

void graph_add_vertex(graph *g, int vertex_id) {
    if (!g || !is_valid_vertex_id(vertex_id)) {
        return;
    }

    graph_ensure_capacity(g, vertex_id);
    if (vertex_id >= g->vertex_capacity) {
        return;
    }

    if (g->present[vertex_id]) {
        return;
    }

    int *id_ptr = malloc(sizeof(int));
    if (!id_ptr) {
        return;
    }
    *id_ptr = vertex_id;

    g->vertex_ptrs[vertex_id] = id_ptr;
    g->present[vertex_id] = true;
    g->vertex_count++;

    if (g->rep == GRAPH_ADJACENCY_LIST && g->adj_lists) {
        if (!g->adj_lists[vertex_id]) {
            g->adj_lists[vertex_id] = create_list(ARRAY_LIST, 4, NULL);
        }
    }
}

void graph_add_edge(graph *g, int from, int to, double weight) {
    if (!g || !is_valid_vertex_id(from) || !is_valid_vertex_id(to)) {
        return;
    }

    graph_add_vertex(g, from);
    graph_add_vertex(g, to);

    if (!graph_has_vertex(g, from) || !graph_has_vertex(g, to)) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        graph_adj_list_add_edge(g, from, to, weight);
    } else {
        graph_adj_matrix_add_edge(g, from, to, weight);
    }
}

void graph_remove_edge(graph *g, int from, int to) {
    if (!g || !graph_has_vertex(g, from) || !graph_has_vertex(g, to)) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        graph_adj_list_remove_edge(g, from, to);
    } else {
        graph_adj_matrix_remove_edge(g, from, to);
    }
}

bool graph_has_edge(const graph *g, int from, int to) {
    if (!g || !graph_has_vertex(g, from) || !graph_has_vertex(g, to)) {
        return false;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        return graph_adj_list_has_edge(g, from, to);
    }
    return graph_adj_matrix_has_edge(g, from, to);
}

double graph_get_edge_weight(const graph *g, int from, int to) {
    if (!g || !graph_has_vertex(g, from) || !graph_has_vertex(g, to)) {
        return GRAPH_NO_EDGE;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        return graph_adj_list_get_edge_weight(g, from, to);
    }
    return graph_adj_matrix_get_edge_weight(g, from, to);
}

list *graph_get_neighbors(const graph *g, int vertex) {
    if (!g || !graph_has_vertex(g, vertex)) {
        return create_list(ARRAY_LIST, 4, NULL);
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        return graph_adj_list_get_neighbors(g, vertex);
    }
    return graph_adj_matrix_get_neighbors(g, vertex);
}

