#include "graph_test.h"

void test_create_Graph() {
  for (unsigned int m = 2; m < 10; ++m) {
    struct graph_t *g1 = createGraph(m, TRIANGULAR);
    assert(g1->num_vertices == 3 * (m * m) - 3 * m + 1);
    assert(g1->num_edges == 9 * (m * m) - 15 * m + 6);
    graph_free(g1);
  }
  for (unsigned int m = 3; m < 10; ++m) {
    struct graph_t *g1 = createGraph(m, CYCLIC);
    assert(g1->num_vertices == 12 * m - 18);
    assert(g1->num_edges == 24 * m - 36);
    graph_free(g1);
  }
  /*
  for (unsigned int m = 6; m < 25; m += 3) {
    struct graph_t *g1 = createGraph(m, HOLEY);
    assert(g1->num_vertices == 2 * (m * m) * (1 / 3) + 18 * m - 48);
    assert(g1->num_edges == 2 * (m * m) + 34 * m - 78);
    graph_free(g1);
  }
  */
}