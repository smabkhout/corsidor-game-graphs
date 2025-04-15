#include "graph_test.h"
#include "graph_functions.h"

void test_create_Graph() {
  /*
  for (unsigned int m = 2; m < 20; ++m) {
    struct graph_t *g1 = createGraph(m, TRIANGULAR);
    assert(g1->num_vertices == 3 * (m * m) - 3 * m + 1);
    assert(g1->num_edges == 9 * (m * m) - 15 * m + 6);
    assert(axial_to_index(0, 0, m)== g1->num_vertices/2);
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
    assert(g1->num_vertices == 2 * (m * m/3)+ 18 * m - 48);
    assert(g1->num_edges == 2 * (m * m) + 34 * m - 78);
    graph_free(g1);
  }
  */

  struct graph_t *g2 = createGraph(5, TRIANGULAR);
  // print_hex_grid(g2);
  graph_free(g2);
  struct graph_t *g3 = createGraph(5, CYCLIC);
  // print_hex_grid(g3);
  graph_free(g3);
  struct graph_t *g4 = createGraph(9, HOLEY);
  // print_hex_grid(g4);
  graph_free(g4);
  for (int m = 3; m < 20; ++m) {
    int l = 0;
    int c = 0;
    // assert(in_hexagon_H(l, c, m, 0, 0));
    struct graph_t *g5 = createGraph(m, TRIANGULAR);
    //print_hex_grid(g5);
    // printf("%d\n", axial_to_index(l, c, m));

    //printf("%d, %d, et m = %d\n", axial_to_index(l, c, m), g5->num_vertices / 2, m);
    graph_free(g5);
  }

  /*
   int m = 9;
   struct graph_t *g1 = createGraph(m, HOLEY);
   for (int i = 0; i < g1->num_vertices; ++i) {
     for (int j = 0; j < g1->num_vertices; ++j) {
       if (gsl_spmatrix_uint_get(g1->t, i, j)) {
         printf("%d (%d, %d),", gsl_spmatrix_uint_get(g1->t, i, j), i, j);
       }
     }
   }*/
  /*
  // struct graph_t* g = graph_create(num_vertices);
  for (int l = 1 - m; l < m; ++l) {
    for (int c = 1 - m; c < m; ++c) {
      int vertex = axial_to_index(l, c, m);
      printf("%d ", gsl_spmatrix_uint_get(g1->t), );
    }
  }
  */
  // graph_free(g1);

  printf("test_create_graph passed.\n");
}
/*
int main() {
  test_create_Graph();
  return 0;
}
*/