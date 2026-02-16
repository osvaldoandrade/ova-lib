#include "../../include/graph.h"
#include "graph_internal.h"

#include "../../include/heap.h"
#include "../../include/matrix.h"
#include "../../include/queue.h"
#include "../../include/stack.h"

#include <stdint.h>
#include <stdlib.h>

static int graph_is_valid_vertex(const graph *g, int v) {
    return g && v >= 0 && v < g->vertex_capacity && g->present && g->present[v];
}

static int graph_first_vertex(const graph *g) {
    if (!g) {
        return -1;
    }
    for (int v = 0; v < g->vertex_capacity; v++) {
        if (graph_is_valid_vertex(g, v)) {
            return v;
        }
    }
    return -1;
}

static list *create_vertex_list(const graph *g) {
    int cap = (g && g->vertex_count > 0) ? g->vertex_count : 4;
    return create_list(ARRAY_LIST, cap, NULL);
}

list *graph_bfs(const graph *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = calloc((size_t)g->vertex_capacity, sizeof(bool));
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 0, NULL);
    if (!visited || !q) {
        free(visited);
        if (q) {
            q->free(q);
        }
        order->free(order);
        return NULL;
    }

    visited[start_vertex] = true;
    q->enqueue(q, (void *)(intptr_t)start_vertex);

    while (!q->is_empty(q)) {
        int v = (int)(intptr_t)q->dequeue(q);
        order->insert(order, g->vertex_ptrs[v], order->size(order));

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                if (!visited[e->to]) {
                    visited[e->to] = true;
                    q->enqueue(q, (void *)(intptr_t)e->to);
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to) || visited[to]) {
                    continue;
                }
                if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                    continue;
                }
                visited[to] = true;
                q->enqueue(q, (void *)(intptr_t)to);
            }
        }
    }

    q->free(q);
    free(visited);
    return order;
}

list *graph_dfs_iterative(const graph *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = calloc((size_t)g->vertex_capacity, sizeof(bool));
    stack *st = create_stack(ARRAY_STACK);
    if (!visited || !st) {
        free(visited);
        if (st) {
            st->free(st);
        }
        order->free(order);
        return NULL;
    }

    st->push(st, (void *)(intptr_t)start_vertex);

    while (!st->is_empty(st)) {
        int v = (int)(intptr_t)st->pop(st);
        if (visited[v]) {
            continue;
        }
        visited[v] = true;
        order->insert(order, g->vertex_ptrs[v], order->size(order));

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = n - 1; i >= 0; i--) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to) || visited[e->to]) {
                    continue;
                }
                st->push(st, (void *)(intptr_t)e->to);
            }
        } else {
            for (int to = g->vertex_capacity - 1; to >= 0; to--) {
                if (!graph_is_valid_vertex(g, to) || visited[to]) {
                    continue;
                }
                if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                    continue;
                }
                st->push(st, (void *)(intptr_t)to);
            }
        }
    }

    st->free(st);
    free(visited);
    return order;
}

static void dfs_recursive_visit(const graph *g, int v, bool *visited, list *order) {
    visited[v] = true;
    order->insert(order, g->vertex_ptrs[v], order->size(order));

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
        int n = adj ? adj->size(adj) : 0;
        for (int i = 0; i < n; i++) {
            graph_edge *e = (graph_edge *)adj->get(adj, i);
            if (!e || !graph_is_valid_vertex(g, e->to) || visited[e->to]) {
                continue;
            }
            dfs_recursive_visit(g, e->to, visited, order);
        }
    } else {
        for (int to = 0; to < g->vertex_capacity; to++) {
            if (!graph_is_valid_vertex(g, to) || visited[to]) {
                continue;
            }
            if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                continue;
            }
            dfs_recursive_visit(g, to, visited, order);
        }
    }
}

list *graph_dfs_recursive(const graph *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = calloc((size_t)g->vertex_capacity, sizeof(bool));
    if (!visited) {
        order->free(order);
        return NULL;
    }

    dfs_recursive_visit(g, start_vertex, visited, order);

    free(visited);
    return order;
}

typedef struct {
    int vertex;
    double dist;
} pq_node;

static int pq_node_cmp(const void *a, const void *b) {
    const pq_node *pa = (const pq_node *)a;
    const pq_node *pb = (const pq_node *)b;

    if (pa->dist < pb->dist) return 1;
    if (pa->dist > pb->dist) return -1;
    return 0;
}

int graph_dijkstra(const graph *g, int start_vertex, vector **out_dist) {
    if (out_dist) {
        *out_dist = NULL;
    }

    if (!g || !out_dist || !graph_is_valid_vertex(g, start_vertex)) {
        return 0;
    }

    vector *dist = create_vector(g->vertex_capacity);
    if (!dist) {
        return 0;
    }
    for (int i = 0; i < dist->size; i++) {
        dist->data[i] = GRAPH_NO_EDGE;
    }
    dist->data[start_vertex] = 0.0;

    heap *pq = create_heap(BINARY_HEAP, g->vertex_count > 0 ? g->vertex_count : 4, pq_node_cmp);
    if (!pq) {
        dist->destroy(dist);
        return 0;
    }

    pq_node *start = malloc(sizeof(pq_node));
    if (!start) {
        pq->free(pq);
        dist->destroy(dist);
        return 0;
    }
    start->vertex = start_vertex;
    start->dist = 0.0;
    pq->put(pq, start);

    while (pq->size(pq) > 0) {
        pq_node *cur = (pq_node *)pq->pop(pq);
        if (!cur) {
            break;
        }

        int v = cur->vertex;
        double d = cur->dist;
        free(cur);

        if (d > dist->data[v]) {
            continue;
        }

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                double nd = dist->data[v] + e->weight;
                if (nd < dist->data[e->to]) {
                    dist->data[e->to] = nd;
                    pq_node *next = malloc(sizeof(pq_node));
                    if (!next) {
                        continue;
                    }
                    next->vertex = e->to;
                    next->dist = nd;
                    pq->put(pq, next);
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                double w = g->adj_matrix[v * g->vertex_capacity + to];
                if (w == GRAPH_NO_EDGE) {
                    continue;
                }
                double nd = dist->data[v] + w;
                if (nd < dist->data[to]) {
                    dist->data[to] = nd;
                    pq_node *next = malloc(sizeof(pq_node));
                    if (!next) {
                        continue;
                    }
                    next->vertex = to;
                    next->dist = nd;
                    pq->put(pq, next);
                }
            }
        }
    }

    while (pq->size(pq) > 0) {
        free(pq->pop(pq));
    }
    pq->free(pq);

    *out_dist = dist;
    return 1;
}

int graph_bellman_ford(const graph *g, int start_vertex, vector **out_dist) {
    if (out_dist) {
        *out_dist = NULL;
    }

    if (!g || !out_dist || !graph_is_valid_vertex(g, start_vertex)) {
        return 0;
    }

    vector *dist = create_vector(g->vertex_capacity);
    if (!dist) {
        return 0;
    }
    for (int i = 0; i < dist->size; i++) {
        dist->data[i] = GRAPH_NO_EDGE;
    }
    dist->data[start_vertex] = 0.0;

    int V = g->vertex_count;
    for (int iter = 0; iter < V - 1; iter++) {
        int updated = 0;

        for (int from = 0; from < g->vertex_capacity; from++) {
            if (!graph_is_valid_vertex(g, from) || dist->data[from] == GRAPH_NO_EDGE) {
                continue;
            }

            if (g->rep == GRAPH_ADJACENCY_LIST) {
                list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
                int n = adj ? adj->size(adj) : 0;
                for (int i = 0; i < n; i++) {
                    graph_edge *e = (graph_edge *)adj->get(adj, i);
                    if (!e || !graph_is_valid_vertex(g, e->to)) {
                        continue;
                    }
                    double nd = dist->data[from] + e->weight;
                    if (nd < dist->data[e->to]) {
                        dist->data[e->to] = nd;
                        updated = 1;
                    }
                }
            } else {
                for (int to = 0; to < g->vertex_capacity; to++) {
                    if (!graph_is_valid_vertex(g, to)) {
                        continue;
                    }
                    double w = g->adj_matrix[from * g->vertex_capacity + to];
                    if (w == GRAPH_NO_EDGE) {
                        continue;
                    }
                    double nd = dist->data[from] + w;
                    if (nd < dist->data[to]) {
                        dist->data[to] = nd;
                        updated = 1;
                    }
                }
            }
        }

        if (!updated) {
            break;
        }
    }

    /* Detect negative cycles reachable from start */
    for (int from = 0; from < g->vertex_capacity; from++) {
        if (!graph_is_valid_vertex(g, from) || dist->data[from] == GRAPH_NO_EDGE) {
            continue;
        }

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                if (dist->data[from] + e->weight < dist->data[e->to]) {
                    dist->destroy(dist);
                    return 0;
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                double w = g->adj_matrix[from * g->vertex_capacity + to];
                if (w == GRAPH_NO_EDGE) {
                    continue;
                }
                if (dist->data[from] + w < dist->data[to]) {
                    dist->destroy(dist);
                    return 0;
                }
            }
        }
    }

    *out_dist = dist;
    return 1;
}

matrix *graph_floyd_warshall(const graph *g) {
    if (!g || g->vertex_capacity <= 0) {
        return NULL;
    }

    matrix *dist = create_matrix(g->vertex_capacity, g->vertex_capacity);
    if (!dist) {
        return NULL;
    }

    for (int i = 0; i < g->vertex_capacity; i++) {
        for (int j = 0; j < g->vertex_capacity; j++) {
            dist->data[i][j] = GRAPH_NO_EDGE;
        }
    }

    for (int v = 0; v < g->vertex_capacity; v++) {
        if (graph_is_valid_vertex(g, v)) {
            dist->data[v][v] = 0.0;
        }
    }

    for (int from = 0; from < g->vertex_capacity; from++) {
        if (!graph_is_valid_vertex(g, from)) {
            continue;
        }

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                if (e->weight < dist->data[from][e->to]) {
                    dist->data[from][e->to] = e->weight;
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                double w = g->adj_matrix[from * g->vertex_capacity + to];
                if (w == GRAPH_NO_EDGE) {
                    continue;
                }
                if (w < dist->data[from][to]) {
                    dist->data[from][to] = w;
                }
            }
        }
    }

    for (int k = 0; k < g->vertex_capacity; k++) {
        if (!graph_is_valid_vertex(g, k)) {
            continue;
        }
        for (int i = 0; i < g->vertex_capacity; i++) {
            if (!graph_is_valid_vertex(g, i) || dist->data[i][k] == GRAPH_NO_EDGE) {
                continue;
            }
            for (int j = 0; j < g->vertex_capacity; j++) {
                if (!graph_is_valid_vertex(g, j) || dist->data[k][j] == GRAPH_NO_EDGE) {
                    continue;
                }
                double nd = dist->data[i][k] + dist->data[k][j];
                if (nd < dist->data[i][j]) {
                    dist->data[i][j] = nd;
                }
            }
        }
    }

    return dist;
}

static int edge_min_heap_cmp(const void *a, const void *b) {
    const graph_weighted_edge *ea = (const graph_weighted_edge *)a;
    const graph_weighted_edge *eb = (const graph_weighted_edge *)b;

    if (ea->weight < eb->weight) return 1;
    if (ea->weight > eb->weight) return -1;
    return 0;
}

static void prim_push_edges(const graph *g, int from, const bool *in_mst, heap *pq) {
    if (!g || !pq || !in_mst) {
        return;
    }

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
        int n = adj ? adj->size(adj) : 0;
        for (int i = 0; i < n; i++) {
            graph_edge *e = (graph_edge *)adj->get(adj, i);
            if (!e || !graph_is_valid_vertex(g, e->to) || in_mst[e->to]) {
                continue;
            }
            graph_weighted_edge *we = malloc(sizeof(graph_weighted_edge));
            if (!we) {
                continue;
            }
            we->from = from;
            we->to = e->to;
            we->weight = e->weight;
            pq->put(pq, we);
        }
        return;
    }

    for (int to = 0; to < g->vertex_capacity; to++) {
        if (!graph_is_valid_vertex(g, to) || in_mst[to]) {
            continue;
        }
        double w = g->adj_matrix[from * g->vertex_capacity + to];
        if (w == GRAPH_NO_EDGE) {
            continue;
        }
        graph_weighted_edge *we = malloc(sizeof(graph_weighted_edge));
        if (!we) {
            continue;
        }
        we->from = from;
        we->to = to;
        we->weight = w;
        pq->put(pq, we);
    }
}

list *graph_mst_prim(const graph *g, int start_vertex) {
    if (!g || g->type != GRAPH_UNDIRECTED) {
        return NULL;
    }

    list *mst = create_list(ARRAY_LIST, g->vertex_count > 0 ? g->vertex_count : 4, NULL);
    if (!mst) {
        return NULL;
    }

    if (g->vertex_count == 0) {
        return mst;
    }

    int start = graph_is_valid_vertex(g, start_vertex) ? start_vertex : graph_first_vertex(g);
    if (start < 0) {
        return mst;
    }

    bool *in_mst = calloc((size_t)g->vertex_capacity, sizeof(bool));
    heap *pq = create_heap(BINARY_HEAP, g->vertex_count > 0 ? g->vertex_count : 4, edge_min_heap_cmp);
    if (!in_mst || !pq) {
        free(in_mst);
        if (pq) {
            pq->free(pq);
        }
        mst->free(mst);
        return NULL;
    }

    in_mst[start] = true;
    prim_push_edges(g, start, in_mst, pq);

    while (pq->size(pq) > 0 && mst->size(mst) < g->vertex_count - 1) {
        graph_weighted_edge *e = (graph_weighted_edge *)pq->pop(pq);
        if (!e) {
            break;
        }
        if (in_mst[e->to]) {
            free(e);
            continue;
        }

        mst->insert(mst, e, mst->size(mst));
        in_mst[e->to] = true;
        prim_push_edges(g, e->to, in_mst, pq);
    }

    while (pq->size(pq) > 0) {
        free(pq->pop(pq));
    }

    pq->free(pq);
    free(in_mst);
    return mst;
}

typedef struct {
    graph_weighted_edge **items;
    int size;
    int cap;
} edge_vec;

static int edge_vec_push(edge_vec *v, graph_weighted_edge *e) {
    if (!v) {
        return 0;
    }
    if (v->size >= v->cap) {
        int new_cap = v->cap > 0 ? v->cap * 2 : 16;
        graph_weighted_edge **n = realloc(v->items, (size_t)new_cap * sizeof(graph_weighted_edge *));
        if (!n) {
            return 0;
        }
        v->items = n;
        v->cap = new_cap;
    }
    v->items[v->size++] = e;
    return 1;
}

static int edge_cmp_qsort(const void *a, const void *b) {
    const graph_weighted_edge *ea = *(graph_weighted_edge *const *)a;
    const graph_weighted_edge *eb = *(graph_weighted_edge *const *)b;
    if (ea->weight < eb->weight) return -1;
    if (ea->weight > eb->weight) return 1;
    return 0;
}

static int uf_find(int *parent, int v) {
    if (parent[v] == v) {
        return v;
    }
    parent[v] = uf_find(parent, parent[v]);
    return parent[v];
}

static void uf_union(int *parent, int *rank, int a, int b) {
    int ra = uf_find(parent, a);
    int rb = uf_find(parent, b);
    if (ra == rb) {
        return;
    }
    if (rank[ra] < rank[rb]) {
        parent[ra] = rb;
    } else if (rank[ra] > rank[rb]) {
        parent[rb] = ra;
    } else {
        parent[rb] = ra;
        rank[ra]++;
    }
}

list *graph_mst_kruskal(const graph *g) {
    if (!g || g->type != GRAPH_UNDIRECTED) {
        return NULL;
    }

    list *mst = create_list(ARRAY_LIST, g->vertex_count > 0 ? g->vertex_count : 4, NULL);
    if (!mst) {
        return NULL;
    }

    edge_vec edges = {0};

    for (int from = 0; from < g->vertex_capacity; from++) {
        if (!graph_is_valid_vertex(g, from)) {
            continue;
        }

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                if (from >= e->to) {
                    continue;
                }
                graph_weighted_edge *we = malloc(sizeof(graph_weighted_edge));
                if (!we) {
                    continue;
                }
                we->from = from;
                we->to = e->to;
                we->weight = e->weight;
                if (!edge_vec_push(&edges, we)) {
                    free(we);
                }
            }
        } else {
            for (int to = from + 1; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                double w = g->adj_matrix[from * g->vertex_capacity + to];
                if (w == GRAPH_NO_EDGE) {
                    continue;
                }
                graph_weighted_edge *we = malloc(sizeof(graph_weighted_edge));
                if (!we) {
                    continue;
                }
                we->from = from;
                we->to = to;
                we->weight = w;
                if (!edge_vec_push(&edges, we)) {
                    free(we);
                }
            }
        }
    }

    qsort(edges.items, (size_t)edges.size, sizeof(graph_weighted_edge *), edge_cmp_qsort);

    int *parent = malloc((size_t)g->vertex_capacity * sizeof(int));
    int *rank = calloc((size_t)g->vertex_capacity, sizeof(int));
    if (!parent || !rank) {
        free(parent);
        free(rank);
        for (int i = 0; i < edges.size; i++) {
            free(edges.items[i]);
        }
        free(edges.items);
        mst->free(mst);
        return NULL;
    }

    for (int v = 0; v < g->vertex_capacity; v++) {
        parent[v] = graph_is_valid_vertex(g, v) ? v : -1;
    }

    for (int i = 0; i < edges.size && mst->size(mst) < g->vertex_count - 1; i++) {
        graph_weighted_edge *e = edges.items[i];
        int ra = uf_find(parent, e->from);
        int rb = uf_find(parent, e->to);
        if (ra != rb) {
            uf_union(parent, rank, ra, rb);
            mst->insert(mst, e, mst->size(mst));
        } else {
            free(e);
        }
        edges.items[i] = NULL;
    }

    for (int i = 0; i < edges.size; i++) {
        free(edges.items[i]);
    }
    free(edges.items);
    free(parent);
    free(rank);

    return mst;
}

list *graph_connected_components(const graph *g) {
    if (!g || g->type != GRAPH_UNDIRECTED) {
        return NULL;
    }

    list *components = create_list(ARRAY_LIST, 4, NULL);
    if (!components) {
        return NULL;
    }

    bool *visited = calloc((size_t)g->vertex_capacity, sizeof(bool));
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 0, NULL);
    if (!visited || !q) {
        free(visited);
        if (q) {
            q->free(q);
        }
        components->free(components);
        return NULL;
    }

    for (int start = 0; start < g->vertex_capacity; start++) {
        if (!graph_is_valid_vertex(g, start) || visited[start]) {
            continue;
        }

        list *comp = create_list(ARRAY_LIST, 4, NULL);
        if (!comp) {
            continue;
        }

        visited[start] = true;
        q->enqueue(q, (void *)(intptr_t)start);

        while (!q->is_empty(q)) {
            int v = (int)(intptr_t)q->dequeue(q);
            comp->insert(comp, g->vertex_ptrs[v], comp->size(comp));

            if (g->rep == GRAPH_ADJACENCY_LIST) {
                list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
                int n = adj ? adj->size(adj) : 0;
                for (int i = 0; i < n; i++) {
                    graph_edge *e = (graph_edge *)adj->get(adj, i);
                    if (!e || !graph_is_valid_vertex(g, e->to)) {
                        continue;
                    }
                    if (!visited[e->to]) {
                        visited[e->to] = true;
                        q->enqueue(q, (void *)(intptr_t)e->to);
                    }
                }
            } else {
                for (int to = 0; to < g->vertex_capacity; to++) {
                    if (!graph_is_valid_vertex(g, to) || visited[to]) {
                        continue;
                    }
                    if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                        continue;
                    }
                    visited[to] = true;
                    q->enqueue(q, (void *)(intptr_t)to);
                }
            }
        }

        components->insert(components, comp, components->size(components));
    }

    q->free(q);
    free(visited);
    return components;
}

static void tarjan_dfs(const graph *g,
                       int v,
                       int *index_counter,
                       int *index,
                       int *lowlink,
                       bool *on_stack,
                       stack *st,
                       list *out_components) {
    index[v] = *index_counter;
    lowlink[v] = *index_counter;
    (*index_counter)++;

    st->push(st, (void *)(intptr_t)v);
    on_stack[v] = true;

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
        int n = adj ? adj->size(adj) : 0;
        for (int i = 0; i < n; i++) {
            graph_edge *e = (graph_edge *)adj->get(adj, i);
            int to = (e && graph_is_valid_vertex(g, e->to)) ? e->to : -1;
            if (to < 0) {
                continue;
            }

            if (index[to] == -1) {
                tarjan_dfs(g, to, index_counter, index, lowlink, on_stack, st, out_components);
                if (lowlink[to] < lowlink[v]) {
                    lowlink[v] = lowlink[to];
                }
            } else if (on_stack[to]) {
                if (index[to] < lowlink[v]) {
                    lowlink[v] = index[to];
                }
            }
        }
    } else {
        for (int to = 0; to < g->vertex_capacity; to++) {
            if (!graph_is_valid_vertex(g, to)) {
                continue;
            }
            if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                continue;
            }

            if (index[to] == -1) {
                tarjan_dfs(g, to, index_counter, index, lowlink, on_stack, st, out_components);
                if (lowlink[to] < lowlink[v]) {
                    lowlink[v] = lowlink[to];
                }
            } else if (on_stack[to]) {
                if (index[to] < lowlink[v]) {
                    lowlink[v] = index[to];
                }
            }
        }
    }

    if (lowlink[v] == index[v]) {
        list *component = create_list(ARRAY_LIST, 4, NULL);
        if (!component) {
            return;
        }

        while (!st->is_empty(st)) {
            int w = (int)(intptr_t)st->pop(st);
            on_stack[w] = false;
            component->insert(component, g->vertex_ptrs[w], component->size(component));
            if (w == v) {
                break;
            }
        }

        out_components->insert(out_components, component, out_components->size(out_components));
    }
}

list *graph_strongly_connected_components(const graph *g) {
    if (!g || g->type != GRAPH_DIRECTED) {
        return NULL;
    }

    list *components = create_list(ARRAY_LIST, 4, NULL);
    if (!components) {
        return NULL;
    }

    int *index = malloc((size_t)g->vertex_capacity * sizeof(int));
    int *lowlink = malloc((size_t)g->vertex_capacity * sizeof(int));
    bool *on_stack = calloc((size_t)g->vertex_capacity, sizeof(bool));
    stack *st = create_stack(ARRAY_STACK);
    if (!index || !lowlink || !on_stack || !st) {
        free(index);
        free(lowlink);
        free(on_stack);
        if (st) {
            st->free(st);
        }
        components->free(components);
        return NULL;
    }

    for (int i = 0; i < g->vertex_capacity; i++) {
        index[i] = -1;
        lowlink[i] = -1;
    }

    int index_counter = 0;
    for (int v = 0; v < g->vertex_capacity; v++) {
        if (!graph_is_valid_vertex(g, v) || index[v] != -1) {
            continue;
        }
        tarjan_dfs(g, v, &index_counter, index, lowlink, on_stack, st, components);
    }

    st->free(st);
    free(index);
    free(lowlink);
    free(on_stack);
    return components;
}

list *graph_topological_sort(const graph *g) {
    if (!g || g->type != GRAPH_DIRECTED) {
        return NULL;
    }

    int *indegree = calloc((size_t)g->vertex_capacity, sizeof(int));
    queue *q = create_queue(QUEUE_TYPE_NORMAL, 0, NULL);
    list *order = create_vertex_list(g);
    if (!indegree || !q || !order) {
        free(indegree);
        if (q) {
            q->free(q);
        }
        if (order) {
            order->free(order);
        }
        return NULL;
    }

    for (int from = 0; from < g->vertex_capacity; from++) {
        if (!graph_is_valid_vertex(g, from)) {
            continue;
        }

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                indegree[e->to]++;
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                if (g->adj_matrix[from * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                    continue;
                }
                indegree[to]++;
            }
        }
    }

    for (int v = 0; v < g->vertex_capacity; v++) {
        if (graph_is_valid_vertex(g, v) && indegree[v] == 0) {
            q->enqueue(q, (void *)(intptr_t)v);
        }
    }

    while (!q->is_empty(q)) {
        int v = (int)(intptr_t)q->dequeue(q);
        order->insert(order, g->vertex_ptrs[v], order->size(order));

        if (g->rep == GRAPH_ADJACENCY_LIST) {
            list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
            int n = adj ? adj->size(adj) : 0;
            for (int i = 0; i < n; i++) {
                graph_edge *e = (graph_edge *)adj->get(adj, i);
                if (!e || !graph_is_valid_vertex(g, e->to)) {
                    continue;
                }
                indegree[e->to]--;
                if (indegree[e->to] == 0) {
                    q->enqueue(q, (void *)(intptr_t)e->to);
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                    continue;
                }
                indegree[to]--;
                if (indegree[to] == 0) {
                    q->enqueue(q, (void *)(intptr_t)to);
                }
            }
        }
    }

    int ok = (order->size(order) == g->vertex_count);

    q->free(q);
    free(indegree);

    if (!ok) {
        order->free(order);
        return NULL;
    }
    return order;
}

static bool has_cycle_directed_dfs(const graph *g, int v, unsigned char *color) {
    color[v] = 1;

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
        int n = adj ? adj->size(adj) : 0;
        for (int i = 0; i < n; i++) {
            graph_edge *e = (graph_edge *)adj->get(adj, i);
            if (!e || !graph_is_valid_vertex(g, e->to)) {
                continue;
            }
            int to = e->to;
            if (color[to] == 1) {
                return true;
            }
            if (color[to] == 0 && has_cycle_directed_dfs(g, to, color)) {
                return true;
            }
        }
    } else {
        for (int to = 0; to < g->vertex_capacity; to++) {
            if (!graph_is_valid_vertex(g, to)) {
                continue;
            }
            if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                continue;
            }
            if (color[to] == 1) {
                return true;
            }
            if (color[to] == 0 && has_cycle_directed_dfs(g, to, color)) {
                return true;
            }
        }
    }

    color[v] = 2;
    return false;
}

static bool has_cycle_undirected_dfs(const graph *g, int v, int parent, bool *visited) {
    visited[v] = true;

    if (g->rep == GRAPH_ADJACENCY_LIST) {
        list *adj = g->adj_lists ? g->adj_lists[v] : NULL;
        int n = adj ? adj->size(adj) : 0;
        for (int i = 0; i < n; i++) {
            graph_edge *e = (graph_edge *)adj->get(adj, i);
            if (!e || !graph_is_valid_vertex(g, e->to)) {
                continue;
            }
            int to = e->to;
            if (!visited[to]) {
                if (has_cycle_undirected_dfs(g, to, v, visited)) {
                    return true;
                }
            } else if (to != parent) {
                return true;
            }
        }
    } else {
        for (int to = 0; to < g->vertex_capacity; to++) {
            if (!graph_is_valid_vertex(g, to)) {
                continue;
            }
            if (g->adj_matrix[v * g->vertex_capacity + to] == GRAPH_NO_EDGE) {
                continue;
            }
            if (!visited[to]) {
                if (has_cycle_undirected_dfs(g, to, v, visited)) {
                    return true;
                }
            } else if (to != parent) {
                return true;
            }
        }
    }

    return false;
}

bool graph_has_cycle(const graph *g) {
    if (!g) {
        return false;
    }

    if (g->type == GRAPH_DIRECTED) {
        unsigned char *color = calloc((size_t)g->vertex_capacity, sizeof(unsigned char));
        if (!color) {
            return false;
        }
        for (int v = 0; v < g->vertex_capacity; v++) {
            if (!graph_is_valid_vertex(g, v) || color[v] != 0) {
                continue;
            }
            if (has_cycle_directed_dfs(g, v, color)) {
                free(color);
                return true;
            }
        }
        free(color);
        return false;
    }

    bool *visited = calloc((size_t)g->vertex_capacity, sizeof(bool));
    if (!visited) {
        return false;
    }
    for (int v = 0; v < g->vertex_capacity; v++) {
        if (!graph_is_valid_vertex(g, v) || visited[v]) {
            continue;
        }
        if (has_cycle_undirected_dfs(g, v, -1, visited)) {
            free(visited);
            return true;
        }
    }
    free(visited);
    return false;
}

