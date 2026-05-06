#include "../../include/heap.h"
#include "../../include/queue.h"
#include "../../include/stack.h"
#include "graph_internal.h"
#include "../matrix/matrix_internal.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int graph_is_valid_vertex(const graph_impl *g, int v) {
    return g && v >= 0 && v < g->vertex_capacity && g->present && g->present[v];
}

static int graph_first_vertex(const graph_impl *g) {
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

static list *create_vertex_list(const graph_impl *g) {
    int cap = (g && g->vertex_count > 0) ? g->vertex_count : 4;
    return create_list(ARRAY_LIST, cap, NULL);
}

list *graph_bfs_impl(const graph_impl *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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
                if (!e || !graph_is_valid_vertex(g, e->to) || visited[e->to]) {
                    continue;
                }
                visited[e->to] = true;
                q->enqueue(q, (void *)(intptr_t)e->to);
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

list *graph_dfs_iterative_impl(const graph_impl *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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

static void dfs_recursive_visit(const graph_impl *g, int v, bool *visited, list *order) {
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

list *graph_dfs_recursive_impl(const graph_impl *g, int start_vertex) {
    list *order = create_vertex_list(g);
    if (!order) {
        return NULL;
    }

    if (!graph_is_valid_vertex(g, start_vertex)) {
        return order;
    }

    bool *visited = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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

typedef struct pq_arena_chunk {
    struct pq_arena_chunk *next;
    int used;
    int capacity;
    pq_node nodes[];
} pq_arena_chunk;

typedef struct {
    pq_arena_chunk *head;
    int next_capacity;
} pq_arena;

static void pq_arena_init(pq_arena *a, int initial_capacity) {
    a->head = NULL;
    a->next_capacity = initial_capacity > 16 ? initial_capacity : 16;
}

static pq_node *pq_arena_alloc(pq_arena *a) {
    if (!a->head || a->head->used >= a->head->capacity) {
        int cap = a->next_capacity;
        pq_arena_chunk *c = (pq_arena_chunk *)malloc(
            sizeof(pq_arena_chunk) + (size_t)cap * sizeof(pq_node));
        if (!c) return NULL;
        c->next = a->head;
        c->used = 0;
        c->capacity = cap;
        a->head = c;
        if (a->next_capacity < (1 << 20)) {
            a->next_capacity = a->next_capacity * 2;
        }
    }
    return &a->head->nodes[a->head->used++];
}

static void pq_arena_destroy(pq_arena *a) {
    pq_arena_chunk *c = a->head;
    while (c) {
        pq_arena_chunk *next = c->next;
        free(c);
        c = next;
    }
    a->head = NULL;
}

int graph_dijkstra_impl(const graph_impl *g, int start_vertex, vector **out_dist) {
    if (out_dist) {
        *out_dist = NULL;
    }
    if (!g || !out_dist || !graph_is_valid_vertex(g, start_vertex)) {
        return 0;
    }

    vector *dist = create_vector(g->vertex_capacity);
    vector_impl *dist_impl = vector_impl_from_vector(dist);
    if (!dist_impl) {
        return 0;
    }

    for (int i = 0; i < dist_impl->size; i++) {
        dist_impl->data[i] = GRAPH_NO_EDGE;
    }
    dist_impl->data[start_vertex] = 0.0;

    heap *pq = create_heap(BINARY_HEAP, g->vertex_count > 0 ? g->vertex_count : 4, pq_node_cmp);
    if (!pq) {
        dist->free(dist);
        return 0;
    }

    pq_arena arena;
    pq_arena_init(&arena, g->vertex_count > 0 ? g->vertex_count : 16);

    pq_node *start = pq_arena_alloc(&arena);
    if (!start) {
        pq_arena_destroy(&arena);
        pq->free(pq);
        dist->free(dist);
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

        if (d > dist_impl->data[v]) {
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
                double nd = dist_impl->data[v] + e->weight;
                if (nd < dist_impl->data[e->to]) {
                    dist_impl->data[e->to] = nd;
                    pq_node *next = pq_arena_alloc(&arena);
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
                double nd = dist_impl->data[v] + w;
                if (nd < dist_impl->data[to]) {
                    dist_impl->data[to] = nd;
                    pq_node *next = pq_arena_alloc(&arena);
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

    pq->free(pq);
    pq_arena_destroy(&arena);

    *out_dist = dist;
    return 1;
}

int graph_bellman_ford_impl(const graph_impl *g, int start_vertex, vector **out_dist) {
    if (out_dist) {
        *out_dist = NULL;
    }
    if (!g || !out_dist || !graph_is_valid_vertex(g, start_vertex)) {
        return 0;
    }

    vector *dist = create_vector(g->vertex_capacity);
    vector_impl *dist_impl = vector_impl_from_vector(dist);
    if (!dist_impl) {
        return 0;
    }

    for (int i = 0; i < dist_impl->size; i++) {
        dist_impl->data[i] = GRAPH_NO_EDGE;
    }
    dist_impl->data[start_vertex] = 0.0;

    int V = g->vertex_count;
    int V_cap = g->vertex_capacity;

    // SPFA-style active-set: only iterate `from` vertices whose distance
    // changed in the previous pass. The first pass starts with just
    // start_vertex; subsequent passes swap the bitmaps.
    char *active = (char *)calloc((size_t)V_cap, 1);
    char *next_active = (char *)calloc((size_t)V_cap, 1);
    if (!active || !next_active) {
        free(active);
        free(next_active);
        dist->free(dist);
        return 0;
    }
    active[start_vertex] = 1;

    for (int iter = 0; iter < V - 1; iter++) {
        int updated = 0;
        memset(next_active, 0, (size_t)V_cap);
        for (int from = 0; from < V_cap; from++) {
            if (!active[from]) {
                continue;
            }

            if (g->rep == GRAPH_ADJACENCY_LIST) {
                list *adj = g->adj_lists ? g->adj_lists[from] : NULL;
                int n = adj ? adj->size(adj) : 0;
                double from_d = dist_impl->data[from];
                for (int i = 0; i < n; i++) {
                    graph_edge *e = (graph_edge *)adj->get(adj, i);
                    if (!e || !graph_is_valid_vertex(g, e->to)) {
                        continue;
                    }
                    double nd = from_d + e->weight;
                    if (nd < dist_impl->data[e->to]) {
                        dist_impl->data[e->to] = nd;
                        next_active[e->to] = 1;
                        updated = 1;
                    }
                }
            } else {
                double from_d = dist_impl->data[from];
                const double *row = &g->adj_matrix[(size_t)from * (size_t)V_cap];
                for (int to = 0; to < V_cap; to++) {
                    if (!graph_is_valid_vertex(g, to)) {
                        continue;
                    }
                    double w = row[to];
                    if (w == GRAPH_NO_EDGE) {
                        continue;
                    }
                    double nd = from_d + w;
                    if (nd < dist_impl->data[to]) {
                        dist_impl->data[to] = nd;
                        next_active[to] = 1;
                        updated = 1;
                    }
                }
            }
        }

        if (!updated) {
            break;
        }
        char *tmp = active;
        active = next_active;
        next_active = tmp;
    }

    free(active);
    free(next_active);

    for (int from = 0; from < g->vertex_capacity; from++) {
        if (!graph_is_valid_vertex(g, from) || dist_impl->data[from] == GRAPH_NO_EDGE) {
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
                if (dist_impl->data[from] + e->weight < dist_impl->data[e->to]) {
                    dist->free(dist);
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
                if (dist_impl->data[from] + w < dist_impl->data[to]) {
                    dist->free(dist);
                    return 0;
                }
            }
        }
    }

    *out_dist = dist;
    return 1;
}

matrix *graph_floyd_warshall_impl(const graph_impl *g) {
    if (!g || g->vertex_capacity <= 0) {
        return NULL;
    }

    matrix *dist = create_matrix(g->vertex_capacity, g->vertex_capacity);
    matrix_impl *dist_impl = matrix_impl_from_matrix(dist);
    if (!dist_impl) {
        return NULL;
    }

    for (int i = 0; i < g->vertex_capacity; i++) {
        for (int j = 0; j < g->vertex_capacity; j++) {
            dist_impl->data[i][j] = GRAPH_NO_EDGE;
        }
    }

    for (int v = 0; v < g->vertex_capacity; v++) {
        if (graph_is_valid_vertex(g, v)) {
            dist_impl->data[v][v] = 0.0;
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
                if (e->weight < dist_impl->data[from][e->to]) {
                    dist_impl->data[from][e->to] = e->weight;
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
                if (w < dist_impl->data[from][to]) {
                    dist_impl->data[from][to] = w;
                }
            }
        }
    }

    int *valid = (int *)malloc((size_t)g->vertex_capacity * sizeof(int));
    if (!valid) {
        dist->free(dist);
        return NULL;
    }
    int valid_count = 0;
    for (int v = 0; v < g->vertex_capacity; v++) {
        if (graph_is_valid_vertex(g, v)) {
            valid[valid_count++] = v;
        }
    }

    for (int ki = 0; ki < valid_count; ki++) {
        int k = valid[ki];
        for (int ii = 0; ii < valid_count; ii++) {
            int i = valid[ii];
            double d_ik = dist_impl->data[i][k];
            if (d_ik == GRAPH_NO_EDGE) {
                continue;
            }
            for (int ji = 0; ji < valid_count; ji++) {
                int j = valid[ji];
                double d_kj = dist_impl->data[k][j];
                if (d_kj == GRAPH_NO_EDGE) {
                    continue;
                }
                double nd = d_ik + d_kj;
                if (nd < dist_impl->data[i][j]) {
                    dist_impl->data[i][j] = nd;
                }
            }
        }
    }

    free(valid);

    return dist;
}

typedef struct {
    int from;
    int to;
    double weight;
} prim_cand;

static int prim_cand_cmp(const void *a, const void *b) {
    const prim_cand *pa = (const prim_cand *)a;
    const prim_cand *pb = (const prim_cand *)b;
    if (pa->weight < pb->weight) return 1;
    if (pa->weight > pb->weight) return -1;
    return 0;
}

typedef struct prim_arena_chunk {
    struct prim_arena_chunk *next;
    int used;
    int capacity;
    prim_cand nodes[];
} prim_arena_chunk;

typedef struct {
    prim_arena_chunk *head;
    int next_capacity;
} prim_arena;

static void prim_arena_init(prim_arena *a, int initial_capacity) {
    a->head = NULL;
    a->next_capacity = initial_capacity > 16 ? initial_capacity : 16;
}

static prim_cand *prim_arena_alloc(prim_arena *a) {
    if (!a->head || a->head->used >= a->head->capacity) {
        int cap = a->next_capacity;
        prim_arena_chunk *c = (prim_arena_chunk *)malloc(
            sizeof(prim_arena_chunk) + (size_t)cap * sizeof(prim_cand));
        if (!c) return NULL;
        c->next = a->head;
        c->used = 0;
        c->capacity = cap;
        a->head = c;
        if (a->next_capacity < (1 << 20)) {
            a->next_capacity = a->next_capacity * 2;
        }
    }
    return &a->head->nodes[a->head->used++];
}

static void prim_arena_destroy(prim_arena *a) {
    prim_arena_chunk *c = a->head;
    while (c) {
        prim_arena_chunk *next = c->next;
        free(c);
        c = next;
    }
    a->head = NULL;
}

static void prim_push_edges(const graph_impl *g, int from, const bool *in_mst,
                            heap *pq, prim_arena *arena) {
    if (!g || !pq || !in_mst || !arena) {
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
            prim_cand *c = prim_arena_alloc(arena);
            if (!c) {
                continue;
            }
            c->from = from;
            c->to = e->to;
            c->weight = e->weight;
            pq->put(pq, c);
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
        prim_cand *c = prim_arena_alloc(arena);
        if (!c) {
            continue;
        }
        c->from = from;
        c->to = to;
        c->weight = w;
        pq->put(pq, c);
    }
}

list *graph_mst_prim_impl(const graph_impl *g, int start_vertex) {
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

    bool *in_mst = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
    heap *pq = create_heap(BINARY_HEAP, g->vertex_count > 0 ? g->vertex_count : 4, prim_cand_cmp);
    if (!in_mst || !pq) {
        free(in_mst);
        if (pq) {
            pq->free(pq);
        }
        mst->free(mst);
        return NULL;
    }

    prim_arena arena;
    prim_arena_init(&arena, g->vertex_count > 0 ? g->vertex_count : 16);

    in_mst[start] = true;
    prim_push_edges(g, start, in_mst, pq, &arena);

    while (pq->size(pq) > 0 && mst->size(mst) < g->vertex_count - 1) {
        prim_cand *c = (prim_cand *)pq->pop(pq);
        if (!c) {
            break;
        }
        if (in_mst[c->to]) {
            continue;
        }

        graph_weighted_edge *we = graph_create_weighted_edge(c->from, c->to, c->weight);
        if (!we) {
            break;
        }
        mst->insert(mst, we, mst->size(mst));
        in_mst[c->to] = true;
        prim_push_edges(g, c->to, in_mst, pq, &arena);
    }

    pq->free(pq);
    prim_arena_destroy(&arena);
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
        graph_weighted_edge **n =
            (graph_weighted_edge **)realloc(v->items, (size_t)new_cap * sizeof(graph_weighted_edge *));
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
    graph_weighted_edge_impl *lhs = graph_weighted_edge_impl_from_public(ea);
    graph_weighted_edge_impl *rhs = graph_weighted_edge_impl_from_public(eb);
    if (!lhs || !rhs) {
        return 0;
    }
    if (lhs->weight < rhs->weight) return -1;
    if (lhs->weight > rhs->weight) return 1;
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

list *graph_mst_kruskal_impl(const graph_impl *g) {
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
                if (!e || !graph_is_valid_vertex(g, e->to) || from >= e->to) {
                    continue;
                }
                graph_weighted_edge *we = graph_create_weighted_edge(from, e->to, e->weight);
                if (we && !edge_vec_push(&edges, we)) {
                    we->free(we);
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
                graph_weighted_edge *we = graph_create_weighted_edge(from, to, w);
                if (we && !edge_vec_push(&edges, we)) {
                    we->free(we);
                }
            }
        }
    }

    qsort(edges.items, (size_t)edges.size, sizeof(graph_weighted_edge *), edge_cmp_qsort);

    int *parent = (int *)malloc((size_t)g->vertex_capacity * sizeof(int));
    int *rank = (int *)calloc((size_t)g->vertex_capacity, sizeof(int));
    if (!parent || !rank) {
        free(parent);
        free(rank);
        for (int i = 0; i < edges.size; i++) {
            if (edges.items[i]) {
                edges.items[i]->free(edges.items[i]);
            }
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
        graph_weighted_edge_impl *edge = graph_weighted_edge_impl_from_public(e);
        if (!edge) {
            continue;
        }
        int ra = uf_find(parent, edge->from);
        int rb = uf_find(parent, edge->to);
        if (ra != rb) {
            uf_union(parent, rank, ra, rb);
            mst->insert(mst, e, mst->size(mst));
            edges.items[i] = NULL;
        } else {
            e->free(e);
            edges.items[i] = NULL;
        }
    }

    for (int i = 0; i < edges.size; i++) {
        if (edges.items[i]) {
            edges.items[i]->free(edges.items[i]);
        }
    }
    free(edges.items);
    free(parent);
    free(rank);

    return mst;
}

list *graph_connected_components_impl(const graph_impl *g) {
    if (!g || g->type != GRAPH_UNDIRECTED) {
        return NULL;
    }

    list *components = create_list(ARRAY_LIST, 4, NULL);
    if (!components) {
        return NULL;
    }

    bool *visited = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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
                    if (!e || !graph_is_valid_vertex(g, e->to) || visited[e->to]) {
                        continue;
                    }
                    visited[e->to] = true;
                    q->enqueue(q, (void *)(intptr_t)e->to);
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

static void tarjan_dfs(const graph_impl *g,
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
            } else if (on_stack[to] && index[to] < lowlink[v]) {
                lowlink[v] = index[to];
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
            } else if (on_stack[to] && index[to] < lowlink[v]) {
                lowlink[v] = index[to];
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

list *graph_strongly_connected_components_impl(const graph_impl *g) {
    if (!g || g->type != GRAPH_DIRECTED) {
        return NULL;
    }

    list *components = create_list(ARRAY_LIST, 4, NULL);
    if (!components) {
        return NULL;
    }

    int *index = (int *)malloc((size_t)g->vertex_capacity * sizeof(int));
    int *lowlink = (int *)malloc((size_t)g->vertex_capacity * sizeof(int));
    bool *on_stack = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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

list *graph_topological_sort_impl(const graph_impl *g) {
    if (!g || g->type != GRAPH_DIRECTED) {
        return NULL;
    }

    int *indegree = (int *)calloc((size_t)g->vertex_capacity, sizeof(int));
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
                if (e && graph_is_valid_vertex(g, e->to)) {
                    indegree[e->to]++;
                }
            }
        } else {
            for (int to = 0; to < g->vertex_capacity; to++) {
                if (!graph_is_valid_vertex(g, to)) {
                    continue;
                }
                if (g->adj_matrix[from * g->vertex_capacity + to] != GRAPH_NO_EDGE) {
                    indegree[to]++;
                }
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

static bool has_cycle_directed_dfs(const graph_impl *g, int v, unsigned char *color) {
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

static bool has_cycle_undirected_dfs(const graph_impl *g, int v, int parent, bool *visited) {
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

bool graph_has_cycle_impl(const graph_impl *g) {
    if (!g) {
        return false;
    }

    if (g->type == GRAPH_DIRECTED) {
        unsigned char *color = (unsigned char *)calloc((size_t)g->vertex_capacity, sizeof(unsigned char));
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

    bool *visited = (bool *)calloc((size_t)g->vertex_capacity, sizeof(bool));
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
