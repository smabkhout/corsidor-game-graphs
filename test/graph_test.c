#include "graph_test.h"

void test_create_Graph(){
     struct graph_t *g1 = createGraph(5, TRIANGULAR);
     struct graph_t *g2 = createGraph(5, CYCLIC);
     assert(g1->num_vertices==61);
     assert(g1->num_edges == 156);
     assert(g2->num_vertices==42);
     assert(g2->num_edges == 84);
}



void test_graph_print(){
    struct graph_t *g1 = createGraph(5, TRIANGULAR);
    struct graph_t *g2 = createGraph(5, CYCLIC);
    graph_print(g1);
    graph_free(g1);
    graph_print(g2);
    graph_free(g2);
}




