#include "graph_test.h"
#include "graph_functions.h"

void test_graph_generate_triangular() {
  for (int m = 3; m < 10; m++) {
    struct graph_t g;
    g.type         = TRIANGULAR;
    g.num_vertices = 0;
    g.num_edges    = 0;

    unsigned int n = 3 * (m * m) - 3 * m + 1;
    g.t            = gsl_spmatrix_uint_alloc(n, n);
    assert(g.t != NULL);

    graph_generate(m, &g, in_hexagon_T);
    assert(g.num_vertices == n);

    assert(g.num_edges > 0);
    assert(g.num_edges <= 3 * g.num_vertices);  // dans un graphe hexagonal

    // Vérifier que toutes les arêtes sont dans l’hexagone
    for (size_t k = 0; k < g.t->nz; ++k) {
      vertex_t     i   = g.t->i[k];
      vertex_t     j   = g.t->p[k];
      unsigned int dir = g.t->data[k];

      assert(dir >= 1 && dir <= 6);

      int li, ci, lj, cj;
      index_to_axial(i, m, &li, &ci, TRIANGULAR);
      index_to_axial(j, m, &lj, &cj, TRIANGULAR);

      assert(in_hexagon_T(li, ci, m, 0, 0));
      assert(in_hexagon_T(lj, cj, m, 0, 0));
    }

    gsl_spmatrix_uint_free(g.t);
  }
  printf("test_graph_generate_triangular passed.\n");
}

void test_create_Graph() {
  for (unsigned int m = 2; m < 20; ++m) {
    struct graph_t *g1 = createGraph(m, TRIANGULAR);
    assert(g1->num_vertices == 3 * (m * m) - 3 * m + 1);
    assert(g1->num_edges == 9 * (m * m) - 15 * m + 6);
    unsigned int a = axial_to_index(0, 0, m, TRIANGULAR);
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

  printf("test_create_graph passed.\n");
}

void test_in_hexagon_T() {
  int total_tests = 0;

  for (int m = 2; m <= 10; ++m) {
    // On teste plusieurs origines pour chaque m
    for (int l0 = -1; l0 <= 1; ++l0) {
      for (int c0 = -1; c0 <= 1; ++c0) {
        // On balaye une zone plus grande que l'hexagone
        for (int l = l0 - m - 1; l <= l0 + m + 1; ++l) {
          for (int c = c0 - m - 1; c <= c0 + m + 1; ++c) {
            int l_rel = l - l0;
            int c_rel = c - c0;

            int expected =
                (abs(l_rel) <= m - 1) && (abs(c_rel) <= m - 1) && (abs(l_rel + c_rel) <= m - 1);

            int result = in_hexagon_T(l, c, m, l0, c0);
            assert(result == expected);
            total_tests++;
          }
        }
      }
    }
  }
  printf("test_in_hexagon passed.\n");
}

void test_objectives() {
  for (int m = 5; m < 10; m++) {
    struct graph_t *g = createGraph(m, TRIANGULAR);
    assert(g != NULL);

    for (unsigned int i = 0; i < g->num_objectives; ++i) {
      vertex_t obj = g->objectives[i];
      int      l, c;
      index_to_axial(obj, m, &l, &c, TRIANGULAR);

      int result = is_objective_or_player(l, c, m, g);

      assert(result == 2);
    }

    graph_free(g);
  }
  printf("test_objectives passed\n");
}
