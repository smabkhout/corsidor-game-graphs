#include "strategie3_test.h"
#include <stdio.h>
#include <assert.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>

void test_create_move() {
  struct edge_t edges[2] = {{0, 1}, {1, 2}};
  struct move_t move     = create_move(BLACK, WALL, 0, edges);
  assert(move.c == BLACK);
  assert(move.t == WALL);
  assert(move.m == 0);
  assert(move.e[0].fr == 0 && move.e[0].to == 1);
  assert(move.e[1].fr == 1 && move.e[1].to == 2);
  printf("test_create_move passed.\n");
}

void test_is_empty_vertice() {
  assert(is_empty_vertice(2, 3) == 1);
  assert(is_empty_vertice(4, 4) == 0);
  printf("test_is_empty_position passed.\n");
}

void test_is_connected() {
  struct graph_t *graph = createGraph(3, TRIANGULAR);
  assert(is_connected(graph, 0, 1) == 1);
  assert(is_connected(graph, 1, 2) == 1);
  assert(is_connected(graph, 0, 2) == 0);
  printf("test_is_connected passed.\n");
  graph_free(graph);
}

void test_is_valid_move() {
  struct graph_t *graph  = createGraph(3, TRIANGULAR);
  struct move_t   move1  = create_move(BLACK, MOVE, 2, NULL);
  int             valid1 = is_valid_move(&move1, graph);
  assert(valid1);

  // Create an invalid move (player moves to vertex 5, which is out of bounds)
  struct move_t move2  = create_move(WHITE, MOVE, 50, NULL);
  int           valid2 = is_valid_move(&move2, graph);
  assert(!valid2);

  // Create a valid wall move (remove edge between vertices 1 and 2)
  struct edge_t edges[2] = {{1, 2}, {2, 1}};
  struct move_t move3    = create_move(BLACK, WALL, 0, edges);
  int           valid3   = is_valid_move(&move3, graph);
  assert(valid3);

  // Create an invalid wall move (edge between vertices 5 and 6, out of bounds)
  struct edge_t edges_invalid[2] = {{5, 6}, {6, 5}};
  struct move_t move4            = create_move(WHITE, WALL, 0, edges_invalid);
  int           valid4           = is_valid_move(&move4, graph);
  assert(valid4);
  printf("test_is_valid_move passed .\n");  // Expect 0 (invalid)
  graph_free(graph);
}

void test_dijkstra() {
  struct graph_t *graph = createGraph(3, TRIANGULAR);
  unsigned int    n     = graph->num_vertices;
  vertex_t        tmp[6];
  int             nb = num_neighbors(graph, 0, tmp, 6);
  assert(nb > 0);
  vertex_t v = tmp[0];
  int      d[n], prev[n], next[n];
  dijistra(graph, 0, v, d, prev, next, n);
  assert(d[0] == 0);
  assert(d[v] == 1);
  assert(prev[v] == 0);
  assert((unsigned int)next[0] == v);
  printf("test_dijkstra passed.\n");
  graph_free(graph);
}

void test_calculate_dist_objectives() {
  int             m     = 3;
  struct graph_t *graph = createGraph(m, TRIANGULAR);
  graph->num_objectives = 2;
  graph->objectives     = realloc(graph->objectives, graph->num_objectives * sizeof(vertex_t));
  graph->objectives[0]  = 0;
  graph->objectives[1]  = 2;

  int num_objectives = graph->num_objectives;
  int distance[num_objectives][num_objectives];
  calculate_dist_objectives(graph, num_objectives, distance, 11);
  for (int i = 0; i < num_objectives; i++) {
    assert(distance[i][i] == 0);
  }
  assert(distance[0][2] == 2);
  printf("test_calculate_dist_objectives passed.\n");
  graph_free(graph);
}

void test_calculate_total_dist() {
  int             m     = 3;
  struct graph_t *graph = createGraph(m, TRIANGULAR);
  graph->num_objectives = 3;
  graph->objectives     = realloc(graph->objectives, graph->num_objectives * sizeof(vertex_t));
  graph->objectives[0]  = 0;
  graph->objectives[1]  = 1;
  graph->objectives[2]  = 4;
  int num_objectives    = graph->num_objectives;
  int distance[num_objectives][num_objectives];
  calculate_dist_objectives(graph, num_objectives, distance, 11);
  int order[]    = {0, 1, 2};
  int total_dist = calculate_total_dist(num_objectives, distance, order);
  assert(total_dist > 0);
  assert(total_dist == distance[0][1] + distance[1][2] + distance[2][0]);
  printf("test_calculate_total_dist passed.\n");
  graph_free(graph);
}

void test_next_permutation() {
  int arr[] = {1, 2, 3};
  int n     = 3;
  assert(next_permutation(arr, n) == 1);
  assert(arr[0] == 1 && arr[1] == 3 && arr[2] == 2);
  assert(next_permutation(arr, n) == 1);
  assert(arr[0] == 2 && arr[1] == 1 && arr[2] == 3);
  assert(next_permutation(arr, n) == 1);
  assert(arr[0] == 2 && arr[1] == 3 && arr[2] == 1);
  assert(next_permutation(arr, n) == 1);
  assert(arr[0] == 3 && arr[1] == 1 && arr[2] == 2);
  assert(next_permutation(arr, n) == 1);
  assert(arr[0] == 3 && arr[1] == 2 && arr[2] == 1);
  assert(next_permutation(arr, n) == 0);  // Il n'y a plus de permutation suivante
  printf("test_next_permutation passed.\n");
}

void test_TSP() {
  int             m     = 3;
  struct graph_t *graph = createGraph(m, TRIANGULAR);
  graph->num_objectives = 3;
  graph->objectives     = realloc(graph->objectives, graph->num_objectives * sizeof(vertex_t));
  graph->objectives[0]  = 0;
  graph->objectives[1]  = 10;
  graph->objectives[2]  = 5;

  unsigned int n_obj = graph->num_objectives;
  int          obj_visited[n_obj];
  for (unsigned int i = 0; i < n_obj; ++i) {
    obj_visited[i] = -1;
  }
  int best_order[n_obj];
  int min_distance = TSP(graph, best_order, obj_visited, graph->num_vertices);
  assert(min_distance >= 0);
  int seen[3] = {0, 0, 0};
  for (unsigned int i = 0; i < n_obj; ++i) {
    assert(best_order[i] >= 0 && best_order[i] < (int)n_obj);
    seen[best_order[i]] = 1;
  }
  for (unsigned int i = 0; i < n_obj; ++i) {
    assert(seen[i]);
  }
  printf("test_TSP passed.\n");
  graph_free(graph);
}

void test_find_closest_objective() {
  struct graph_t graph;
  graph.num_vertices    = 5;
  graph.num_objectives  = 2;
  vertex_t objectives[] = {3, 4};
  graph.objectives      = objectives;
  gsl_spmatrix_uint *t  = gsl_spmatrix_uint_alloc(5, 5);
  gsl_spmatrix_uint_set(t, 0, 1, 1);
  gsl_spmatrix_uint_set(t, 1, 2, 1);
  gsl_spmatrix_uint_set(t, 2, 3, 1);
  gsl_spmatrix_uint_set(t, 3, 4, 1);
  graph.t                    = t;
  vertex_t player_pos        = 0;
  vertex_t closest_objective = find_closest_objective(&graph, player_pos, 11);
  // On attend que le plus proche objectif soit le sommet 3  car 0 -> 1 -> 2 -> 3 est le chemin le
  // plus court
  assert(closest_objective == 3);
  printf("Test Passed: Closest objective found correctly.\n");
  gsl_spmatrix_uint_free(t);
}

void test_num_neighbors() {
  struct graph_t *graph = createGraph(3, TRIANGULAR);
  vertex_t        tmp[6];
  int             nb = num_neighbors(graph, 0, tmp, 6);
  assert(nb > 0 && nb <= 6);
  for (int i = 0; i < nb; i++) {
    assert(tmp[i] < graph->num_vertices);
    assert(gsl_spmatrix_uint_get(graph->t, 0, tmp[i]) > 0);
  }
  printf("test_num_neighbors passed.\n");
  graph_free(graph);
}

void test_try_place_wall_basic() {
  struct graph_t *graph = createGraph(3, TRIANGULAR);
  unsigned int    n     = graph->num_vertices;
  vertex_t        tmp[6];
  vertex_t        pos_enemy = (vertex_t)-1;
  int             nb;

  for (vertex_t v = 0; v < n; ++v) {
    nb = num_neighbors(graph, v, tmp, 6);
    if (nb >= 2) {
      pos_enemy = v;
      break;
    }
  }
  assert(pos_enemy != (vertex_t)-1);
  vertex_t next_enemy = tmp[0];

  struct move_t fallback = create_move(NO_COLOR, MOVE, 999, NULL);
  struct move_t mv       = try_place_wall(graph, pos_enemy, next_enemy, fallback);

  assert(mv.t == WALL);
  assert(mv.e[0].fr == pos_enemy && mv.e[0].to == next_enemy);
  assert(mv.e[1].fr == pos_enemy && mv.e[1].to != next_enemy);

  printf("test_try_place_wall_basic passed.\n");
  graph_free(graph);
}