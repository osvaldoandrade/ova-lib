#include "base_test.h"
#include "../include/graph.h"
#include "../include/matrix.h"

static int list_matches(const char *label, list *lst, const int *expected, int expected_len) {
    if (!lst) {
        print_test_result(0, label);
        return 0;
    }

    if (lst->size(lst) != expected_len) {
        print_test_result(0, label);
        return 0;
    }

    for (int i = 0; i < expected_len; i++) {
        int *id = (int *)lst->get(lst, i);
        if (!id || *id != expected[i]) {
            print_test_result(0, label);
            return 0;
        }
    }

    print_test_result(1, label);
    return 1;
}

static void free_mst_edges(list *edges) {
    if (!edges) {
        return;
    }
    int n = edges->size(edges);
    for (int i = 0; i < n; i++) {
        free(edges->get(edges, i));
    }
    edges->free(edges);
}

static double mst_total_weight(list *edges) {
    double sum = 0.0;
    if (!edges) {
        return sum;
    }
    int n = edges->size(edges);
    for (int i = 0; i < n; i++) {
        graph_weighted_edge *e = (graph_weighted_edge *)edges->get(edges, i);
        if (e) {
            sum += e->weight;
        }
    }
    return sum;
}

static graph *build_unweighted_sample(graph_representation rep) {
    graph *g = create_graph(GRAPH_UNDIRECTED, rep);
    if (!g) {
        return NULL;
    }

    for (int i = 0; i <= 4; i++) {
        graph_add_vertex(g, i);
    }

    graph_add_edge(g, 0, 1, 1.0);
    graph_add_edge(g, 0, 2, 1.0);
    graph_add_edge(g, 1, 3, 1.0);
    graph_add_edge(g, 2, 3, 1.0);
    graph_add_edge(g, 3, 4, 1.0);
    return g;
}

static graph *build_weighted_directed_sample(graph_representation rep) {
    graph *g = create_graph(GRAPH_DIRECTED, rep);
    if (!g) {
        return NULL;
    }

    graph_add_edge(g, 0, 1, 1.0);
    graph_add_edge(g, 0, 2, 4.0);
    graph_add_edge(g, 1, 2, 2.0);
    graph_add_edge(g, 1, 3, 5.0);
    graph_add_edge(g, 2, 3, 1.0);
    return g;
}

void test_bfs_and_dfs(graph_representation rep) {
    graph *g = build_unweighted_sample(rep);
    assert_not_null(g);

    int bfs_expected[] = {0, 1, 2, 3, 4};
    list *bfs = graph_bfs(g, 0);
    list_matches("BFS visit order", bfs, bfs_expected, 5);
    bfs->free(bfs);

    int dfs_expected[] = {0, 1, 3, 2, 4};
    list *dfs_it = graph_dfs_iterative(g, 0);
    list_matches("DFS iterative visit order", dfs_it, dfs_expected, 5);
    dfs_it->free(dfs_it);

    list *dfs_rec = graph_dfs_recursive(g, 0);
    list_matches("DFS recursive visit order", dfs_rec, dfs_expected, 5);
    dfs_rec->free(dfs_rec);

    graph_free(g);
}

void test_shortest_paths(graph_representation rep) {
    graph *g = build_weighted_directed_sample(rep);
    assert_not_null(g);

    vector *dist = NULL;
    print_test_result(graph_dijkstra(g, 0, &dist) == 1 && dist != NULL, "Dijkstra returns distances");
    if (dist) {
        print_test_result(dist->data[0] == 0.0 && dist->data[1] == 1.0 && dist->data[2] == 3.0 && dist->data[3] == 4.0,
                          "Dijkstra distances are correct");
        dist->destroy(dist);
    }

    vector *bf = NULL;
    print_test_result(graph_bellman_ford(g, 0, &bf) == 1 && bf != NULL, "Bellman-Ford returns distances");
    if (bf) {
        print_test_result(bf->data[3] == 4.0, "Bellman-Ford distance to vertex 3 is correct");
        bf->destroy(bf);
    }

    matrix *fw = graph_floyd_warshall(g);
    print_test_result(fw != NULL, "Floyd-Warshall returns a matrix");
    if (fw) {
        print_test_result(fw->data[0][3] == 4.0, "Floyd-Warshall all-pairs distance 0->3 is correct");
        fw->destroy(fw);
    }

    graph_free(g);
}

void test_bellman_ford_negative_cycle(graph_representation rep) {
    graph *g = create_graph(GRAPH_DIRECTED, rep);
    assert_not_null(g);

    graph_add_edge(g, 0, 1, 1.0);
    graph_add_edge(g, 1, 2, -1.0);
    graph_add_edge(g, 2, 1, -1.0); /* negative cycle reachable from 0 */

    vector *dist = NULL;
    print_test_result(graph_bellman_ford(g, 0, &dist) == 0, "Bellman-Ford detects reachable negative cycle");
    if (dist) {
        dist->destroy(dist);
    }

    graph_free(g);
}

void test_mst(graph_representation rep) {
    graph *g = create_graph(GRAPH_UNDIRECTED, rep);
    assert_not_null(g);

    graph_add_edge(g, 0, 1, 1.0);
    graph_add_edge(g, 1, 2, 2.0);
    graph_add_edge(g, 0, 2, 3.0);

    list *prim = graph_mst_prim(g, 0);
    print_test_result(prim != NULL && prim->size(prim) == 2, "Prim returns |V|-1 edges");
    if (prim) {
        print_test_result(mst_total_weight(prim) == 3.0, "Prim MST total weight is correct");
        free_mst_edges(prim);
    }

    list *kruskal = graph_mst_kruskal(g);
    print_test_result(kruskal != NULL && kruskal->size(kruskal) == 2, "Kruskal returns |V|-1 edges");
    if (kruskal) {
        print_test_result(mst_total_weight(kruskal) == 3.0, "Kruskal MST total weight is correct");
        free_mst_edges(kruskal);
    }

    graph_free(g);
}

void test_connectivity_and_scc(graph_representation rep) {
    /* Connected components (undirected) */
    graph *u = create_graph(GRAPH_UNDIRECTED, rep);
    assert_not_null(u);

    for (int i = 0; i <= 5; i++) {
        graph_add_vertex(u, i);
    }
    graph_add_edge(u, 0, 1, 1.0);
    graph_add_edge(u, 1, 2, 1.0);
    graph_add_edge(u, 3, 4, 1.0);

    list *comps = graph_connected_components(u);
    print_test_result(comps != NULL && comps->size(comps) == 3, "Connected components returns expected number of components");
    if (comps) {
        int sizes[3] = {0, 0, 0};
        for (int i = 0; i < comps->size(comps) && i < 3; i++) {
            list *c = (list *)comps->get(comps, i);
            sizes[i] = c ? c->size(c) : 0;
        }
        /* order is not guaranteed; check multiset {3,2,1} */
        int ok = 0;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    if (i != j && j != k && i != k && sizes[i] == 3 && sizes[j] == 2 && sizes[k] == 1) {
                        ok = 1;
                    }
                }
            }
        }
        print_test_result(ok, "Connected components sizes match expected");

        for (int i = 0; i < comps->size(comps); i++) {
            list *c = (list *)comps->get(comps, i);
            if (c) {
                c->free(c);
            }
        }
        comps->free(comps);
    }
    graph_free(u);

    /* Strongly connected components (directed) */
    graph *d = create_graph(GRAPH_DIRECTED, rep);
    assert_not_null(d);

    graph_add_edge(d, 0, 1, 1.0);
    graph_add_edge(d, 1, 2, 1.0);
    graph_add_edge(d, 2, 0, 1.0);
    graph_add_edge(d, 2, 3, 1.0);
    graph_add_edge(d, 3, 4, 1.0);
    graph_add_edge(d, 4, 3, 1.0);
    graph_add_vertex(d, 5); /* singleton SCC */

    list *scc = graph_strongly_connected_components(d);
    print_test_result(scc != NULL && scc->size(scc) == 3, "SCC returns expected number of components");
    if (scc) {
        for (int i = 0; i < scc->size(scc); i++) {
            list *c = (list *)scc->get(scc, i);
            if (c) {
                c->free(c);
            }
        }
        scc->free(scc);
    }

    graph_free(d);
}

void test_toposort_and_cycles(graph_representation rep) {
    graph *dag = create_graph(GRAPH_DIRECTED, rep);
    assert_not_null(dag);

    graph_add_edge(dag, 0, 1, 1.0);
    graph_add_edge(dag, 0, 2, 1.0);
    graph_add_edge(dag, 1, 3, 1.0);
    graph_add_edge(dag, 2, 3, 1.0);

    print_test_result(!graph_has_cycle(dag), "DAG has no cycle");
    list *topo = graph_topological_sort(dag);
    print_test_result(topo != NULL && topo->size(topo) == 4, "Topological sort returns all vertices");
    if (topo) {
        int pos[8];
        for (int i = 0; i < 8; i++) pos[i] = -1;
        for (int i = 0; i < topo->size(topo); i++) {
            int *id = (int *)topo->get(topo, i);
            if (id) pos[*id] = i;
        }
        int ok = pos[0] < pos[1] && pos[0] < pos[2] && pos[1] < pos[3] && pos[2] < pos[3];
        print_test_result(ok, "Topological ordering respects edges");
        topo->free(topo);
    }

    graph_free(dag);

    graph *cyc = create_graph(GRAPH_DIRECTED, rep);
    assert_not_null(cyc);
    graph_add_edge(cyc, 0, 1, 1.0);
    graph_add_edge(cyc, 1, 2, 1.0);
    graph_add_edge(cyc, 2, 0, 1.0);
    print_test_result(graph_has_cycle(cyc), "Directed cycle is detected");
    list *topo2 = graph_topological_sort(cyc);
    print_test_result(topo2 == NULL, "Topological sort returns NULL on cyclic graph");
    if (topo2) {
        topo2->free(topo2);
    }
    graph_free(cyc);
}

void run_all_graph_algorithm_tests() {
    test_bfs_and_dfs(GRAPH_ADJACENCY_LIST);
    test_bfs_and_dfs(GRAPH_ADJACENCY_MATRIX);

    test_shortest_paths(GRAPH_ADJACENCY_LIST);
    test_shortest_paths(GRAPH_ADJACENCY_MATRIX);
    test_bellman_ford_negative_cycle(GRAPH_ADJACENCY_LIST);
    test_bellman_ford_negative_cycle(GRAPH_ADJACENCY_MATRIX);

    test_mst(GRAPH_ADJACENCY_LIST);
    test_mst(GRAPH_ADJACENCY_MATRIX);

    test_connectivity_and_scc(GRAPH_ADJACENCY_LIST);
    test_connectivity_and_scc(GRAPH_ADJACENCY_MATRIX);

    test_toposort_and_cycles(GRAPH_ADJACENCY_LIST);
    test_toposort_and_cycles(GRAPH_ADJACENCY_MATRIX);
}

int main() {
    run_all_graph_algorithm_tests();
    return 0;
}

