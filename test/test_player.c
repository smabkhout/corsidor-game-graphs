#include "test_player.h"
#include <stdio.h>
#include <assert.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>

struct graph_t* make_test_graph(int size) {
  struct graph_t* g = malloc(sizeof(struct graph_t));
  g->num_vertices   = size;
  g->num_edges      = 0;
  g->type           = TRIANGULAR;
  g->num_objectives = 0;
  g->start[0]       = 0;
  g->start[1]       = 1;
  g->t              = gsl_spmatrix_uint_alloc(size, size);
  g->objectives     = NULL;
  return g;
}

void test_neighbors_basic() {
  struct graph_t* g = make_test_graph(3);
  gsl_spmatrix_uint_set(g->t, 0, 1, 1);
  gsl_spmatrix_uint_set(g->t, 0, 2, 1);

  vertex_t out[4];
  int      count = get_neighbors(g, 0, out, 4);

  assert(count == 2);
  assert((out[0] == 1 && out[1] == 2) || (out[0] == 2 && out[1] == 1));

  graph_free(g);
  printf("test_neighbors_basic \n");
}

void test_max_out_zero() {
  struct graph_t* g = make_test_graph(3);
  gsl_spmatrix_uint_set(g->t, 0, 1, 1);

  vertex_t out[1];
  int      count = get_neighbors(g, 0, out, 0);

  assert(count == 0);

  graph_free(g);
  printf("test_max_out_zero \n");
}

void test_no_neighbors() {
  struct graph_t* g = make_test_graph(3);

  vertex_t out[2];
  int      count = get_neighbors(g, 1, out, 2);

  assert(count == 0);

  graph_free(g);
  printf("test_no_neighbors \n");
}

void test_limit_max_out() {
  struct graph_t* g = make_test_graph(4);
  gsl_spmatrix_uint_set(g->t, 0, 1, 1);
  gsl_spmatrix_uint_set(g->t, 0, 2, 1);
  gsl_spmatrix_uint_set(g->t, 0, 3, 1);

  vertex_t out[2];
  int      count = get_neighbors(g, 0, out, 2);

  assert(count == 2);
  assert(out[0] != out[1]);

  graph_free(g);
  printf("test_limit_max_out \n");
}

void test_invalid_vertex() {
  struct graph_t* g = make_test_graph(3);
  vertex_t        out[2];

  int count = get_neighbors(g, 999, out, 2);
  assert(count == 0);

  graph_free(g);
  printf("test_invalid_vertex \n");
}

int  visited_objectives[10] = {0};
void free_graph(struct graph_t* g) {
  if (g->t != NULL)
    gsl_spmatrix_uint_free(g->t);
  if (g->objectives != NULL)
    free(g->objectives);
  g->t          = NULL;
  g->objectives = NULL;
}

struct graph_t init_graph_example() {
  struct graph_t g;
  g.num_vertices = 7;
  g.t            = gsl_spmatrix_uint_alloc(g.num_vertices, g.num_vertices);
  gsl_spmatrix_uint_set(g.t, 0, 1, E);
  gsl_spmatrix_uint_set(g.t, 1, 2, E);
  gsl_spmatrix_uint_set(g.t, 2, 3, E);

  gsl_spmatrix_uint_set(g.t, 0, 5, NW);
  gsl_spmatrix_uint_set(g.t, 1, 0, W);
  gsl_spmatrix_uint_set(g.t, 2, 1, W);
  gsl_spmatrix_uint_set(g.t, 3, 2, W);
  gsl_spmatrix_uint_set(g.t, 4, 5, W);

  g.objectives     = malloc(3 * sizeof(vertex_t));
  g.objectives[0]  = 1;
  g.objectives[1]  = 3;
  g.objectives[2]  = 4;
  g.num_objectives = 3;

  return g;
}

void test_is_path_clear() {
  struct graph_t g = init_graph_example();
  vertex_t       result;
  int            ok = is_path_clear(&g, 0, E, 3, 99, &result);
  assert(ok == 1 && result == 3);
  ok = is_path_clear(&g, 0, E, 3, 2, &result);
  assert(ok == 0);
  ok = is_path_clear(&g, 0, W, 1, 99, &result);
  assert(ok == 0);
  printf(" test_is_path_clear OK\n");
  free_graph(&g);
}

void test_get_side_dir_30() {
  enum dir_t d1, d2;

  get_side_dir_30(E, &d1, &d2);
  assert(d1 == NE && d2 == SE);

  get_side_dir_30(NW, &d1, &d2);
  assert(d1 == W && d2 == NE);

  get_side_dir_30(NO_EDGE, &d1, &d2);
  assert(d1 == NO_EDGE && d2 == NO_EDGE);

  printf(" test_get_side_dir_30 OK\n");
}

void test_find_best_move() {
  struct graph_t g = init_graph_example();
  struct move_t  m = find_best_move(&g, 0, 99, E, BLACK);
  assert(m.t == MOVE && m.m == 3);
  m = find_best_move(&g, 0, 2, E, BLACK);
  assert(m.t == MOVE && (m.m == 2 || m.m == 1));
  m = find_best_move(&g, 0, 1, NO_EDGE, BLACK);
  assert(m.t == MOVE);
  printf(" test_find_best_move OK\n");
  free_graph(&g);
}

/*void test_get_next_closest_objective() {
    struct graph_t g = init_graph_example();
    g.objectives[0] = 3;
    g.objectives[1] = 4;
    g.num_objectives = 2;

    visited_objectives[3] = 0;
    visited_objectives[4] = 0;
    vertex_t next = get_next_closest_objective(&g, 0);
    assert(next == 3);

    visited_objectives[3] = 1;
    next = get_next_closest_objective(&g, 0);
    assert(next == 4);

    visited_objectives[4] = 1;
    next = get_next_closest_objective(&g, 0);
    assert(next == 0);  // renvoie position courante

    printf("test_get_next_closest_objective OK\n");
    free_graph(&g);
}


void test_all_objectives_visited() {
    struct graph_t g = init_graph_example();
    g.objectives[0] = 1;
    g.objectives[1] = 2;
    g.num_objectives = 2;
    visited_objectives[1] = 1;
    visited_objectives[2] = 1;
    assert(all_objectives_visited(&g) == 1);
    visited_objectives[2] = 0;
    assert(all_objectives_visited(&g) == 0);
    g.num_objectives = 0;
    assert(all_objectives_visited(&g) == 1); // aucun
    printf(" test_all_objectives_visited OK\n");
    free_graph(&g);
}*/
