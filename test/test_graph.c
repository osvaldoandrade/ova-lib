#include "base_test.h"
#include "../include/graph.h"

static int list_contains_vertex(list *lst, int v) {
    if (!lst) {
        return 0;
    }
    int n = lst->size(lst);
    for (int i = 0; i < n; i++) {
        int *id = (int *)lst->get(lst, i);
        if (id && *id == v) {
            return 1;
        }
    }
    return 0;
}

void test_graph_add_edge_implicit_vertices(graph_representation rep) {
    graph *g = create_graph(GRAPH_DIRECTED, rep);
    assert_not_null(g);

    graph_add_edge(g, 5, 6, 2.0);
    print_test_result(graph_vertex_count(g) == 2, "graph_add_edge implicitly adds missing vertices");
    print_test_result(graph_has_edge(g, 5, 6), "directed edge exists after add");
    print_test_result(!graph_has_edge(g, 6, 5), "reverse edge should not exist in directed graph");

    graph_free(g);
}

void test_graph_basic_undirected(graph_representation rep) {
    graph *g = create_graph(GRAPH_UNDIRECTED, rep);
    assert_not_null(g);

    graph_add_vertex(g, 0);
    graph_add_vertex(g, 1);
    graph_add_vertex(g, 2);

    graph_add_edge(g, 0, 1, 1.5);
    print_test_result(graph_has_edge(g, 0, 1) && graph_has_edge(g, 1, 0), "undirected add_edge creates symmetric edges");
    print_test_result(graph_get_edge_weight(g, 0, 1) == 1.5, "edge weight is stored (0->1)");

    graph_remove_edge(g, 0, 1);
    print_test_result(!graph_has_edge(g, 0, 1) && !graph_has_edge(g, 1, 0), "graph_remove_edge removes symmetric edges");

    graph_add_edge(g, 0, 1, 1.0);
    graph_add_edge(g, 0, 2, 1.0);
    graph_add_edge(g, 0, 3, 1.0); /* also tests implicit vertex addition */

    list *neighbors = graph_get_neighbors(g, 0);
    assert_not_null(neighbors);
    print_test_result(neighbors->size(neighbors) == 3, "graph_get_neighbors returns expected neighbor count");
    print_test_result(list_contains_vertex(neighbors, 1) && list_contains_vertex(neighbors, 2) && list_contains_vertex(neighbors, 3),
                      "graph_get_neighbors contains expected vertices");
    neighbors->free(neighbors);

    graph_free(g);
}

void run_all_graph_tests() {
    test_graph_add_edge_implicit_vertices(GRAPH_ADJACENCY_LIST);
    test_graph_add_edge_implicit_vertices(GRAPH_ADJACENCY_MATRIX);
    test_graph_basic_undirected(GRAPH_ADJACENCY_LIST);
    test_graph_basic_undirected(GRAPH_ADJACENCY_MATRIX);
}

int main() {
    run_all_graph_tests();
    return 0;
}

