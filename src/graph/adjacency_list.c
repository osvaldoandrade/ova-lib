#include "graph_internal.h"

#include <stdlib.h>

static list *ensure_adj_list(graph *g, int vertex) {
    if (!g || !g->adj_lists) {
        return NULL;
    }

    if (!g->adj_lists[vertex]) {
        g->adj_lists[vertex] = create_list(ARRAY_LIST, 4, NULL);
    }
    return g->adj_lists[vertex];
}

static graph_edge *find_edge(list *adj, int to, int *out_index) {
    if (out_index) {
        *out_index = -1;
    }

    if (!adj || !adj->size || !adj->get) {
        return NULL;
    }

    int n = adj->size(adj);
    for (int i = 0; i < n; i++) {
        graph_edge *e = (graph_edge *)adj->get(adj, i);
        if (e && e->to == to) {
            if (out_index) {
                *out_index = i;
            }
            return e;
        }
    }
    return NULL;
}

static void add_one_edge(graph *g, int from, int to, double weight) {
    list *adj = ensure_adj_list(g, from);
    if (!adj) {
        return;
    }

    graph_edge *existing = find_edge(adj, to, NULL);
    if (existing) {
        existing->weight = weight;
        return;
    }

    graph_edge *e = malloc(sizeof(graph_edge));
    if (!e) {
        return;
    }
    e->to = to;
    e->weight = weight;

    adj->insert(adj, e, adj->size(adj));
}

static void remove_one_edge(graph *g, int from, int to) {
    if (!g || !g->adj_lists) {
        return;
    }

    list *adj = g->adj_lists[from];
    if (!adj) {
        return;
    }

    int idx = -1;
    graph_edge *e = find_edge(adj, to, &idx);
    if (!e || idx < 0) {
        return;
    }

    free(e);
    adj->remove(adj, idx);
}

void graph_adj_list_add_edge(graph *g, int from, int to, double weight) {
    if (!g) {
        return;
    }

    add_one_edge(g, from, to, weight);
    if (g->type == GRAPH_UNDIRECTED && from != to) {
        add_one_edge(g, to, from, weight);
    }
}

void graph_adj_list_remove_edge(graph *g, int from, int to) {
    if (!g) {
        return;
    }

    remove_one_edge(g, from, to);
    if (g->type == GRAPH_UNDIRECTED && from != to) {
        remove_one_edge(g, to, from);
    }
}

bool graph_adj_list_has_edge(const graph *g, int from, int to) {
    if (!g || !g->adj_lists) {
        return false;
    }

    list *adj = g->adj_lists[from];
    return find_edge(adj, to, NULL) != NULL;
}

double graph_adj_list_get_edge_weight(const graph *g, int from, int to) {
    if (!g || !g->adj_lists) {
        return GRAPH_NO_EDGE;
    }

    list *adj = g->adj_lists[from];
    graph_edge *e = find_edge(adj, to, NULL);
    return e ? e->weight : GRAPH_NO_EDGE;
}

list *graph_adj_list_get_neighbors(const graph *g, int vertex) {
    list *neighbors = create_list(ARRAY_LIST, 4, NULL);
    if (!neighbors) {
        return NULL;
    }

    if (!g || !g->adj_lists || vertex < 0 || vertex >= g->vertex_capacity) {
        return neighbors;
    }

    list *adj = g->adj_lists[vertex];
    if (!adj) {
        return neighbors;
    }

    int n = adj->size(adj);
    for (int i = 0; i < n; i++) {
        graph_edge *e = (graph_edge *)adj->get(adj, i);
        if (!e) {
            continue;
        }
        if (e->to < 0 || e->to >= g->vertex_capacity || !g->present[e->to]) {
            continue;
        }
        neighbors->insert(neighbors, g->vertex_ptrs[e->to], neighbors->size(neighbors));
    }
    return neighbors;
}

void graph_adj_list_free_edges(graph *g) {
    if (!g || !g->adj_lists) {
        return;
    }

    for (int v = 0; v < g->vertex_capacity; v++) {
        list *adj = g->adj_lists[v];
        if (!adj) {
            continue;
        }

        int n = adj->size(adj);
        for (int i = 0; i < n; i++) {
            free(adj->get(adj, i));
        }
        adj->free(adj);
        g->adj_lists[v] = NULL;
    }
}

