#include "graph_test.h"
#include "graph_functions.h"

void test_create_Graph() {
  for (unsigned int m = 2; m < 20; ++m) {
    struct graph_t *g1 = createGraph(m, TRIANGULAR);
    assert(g1->num_vertices == 3 * (m * m) - 3 * m + 1);
    assert(g1->num_edges == 9 * (m * m) - 15 * m + 6);
    unsigned int a = axial_to_index(0, 0, m);
    assert(a == g1->num_vertices / 2);
    graph_free(g1);
  }
  for (unsigned int m = 3; m < 10; ++m) {
    struct graph_t *g1 = createGraph(m, CYCLIC);
    assert(g1->num_vertices == 12 * m - 18);
    assert(g1->num_edges == 24 * m - 36);
    graph_free(g1);
  }

  for (unsigned int m = 6; m < 25; m += 3) {
    struct graph_t *g1 = createGraph(m, HOLEY);
    assert(g1->num_vertices == 2 * (m * m / 3) + 18 * m - 48);
    assert(g1->num_edges == 2 * (m * m) + 34 * m - 78);
    graph_free(g1);
  }
/*
  struct graph_t *g2 = createGraph(5, TRIANGULAR);
  print_hex_grid(g2);
  graph_free(g2);
  struct graph_t *g3 = createGraph(5, CYCLIC);
  print_hex_grid(g3);
  graph_free(g3);

  for (int m = 6; m < 25; m += 3) {
    int l = 0;
    int c = 0;
    struct graph_t *g5 = createGraph(m, HOLEY);
    print_hex_grid(g5);

    printf("L'origine est à la position d'indice : %d, Nbre de vertices : %d, "
           "etdoit etre : %d, "
           "et m = %d\n",
           axial_to_index(l, c, m), g5->num_vertices,
           2 * (m * m / 3) + 18 * m - 48, m);
    graph_free(g5);
  }
*/
  printf("test_create_graph passed.\n");
}
/*
int main() {
  test_create_Graph();
  return 0;
}
  */
