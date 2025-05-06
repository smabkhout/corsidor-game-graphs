#include <stdio.h>
#include "graph_test.h"
#include "strategie3_test.h"
#include "move2_test.h"
#include "test_player.h"

int main() {
  test_graph_generate_triangular();
  test_create_Graph();
  test_create_move();
  test_in_hexagon_T();
  test_objectives();
  test_is_empty_vertice();
  test_is_connected();
  test_is_valid_move();
  test_dijkstra();
  test_calculate_dist_objectives();
  test_calculate_total_dist();
  test_next_permutation();
  // test_TSP() ;
  test_find_closest_objective();
  test_index_axial_inverse();
  test_direction_axial();
  test_valid_wall();
  test_valid_move();

  test_dijkstra2();
  test_neighbors_basic();
  test_max_out_zero();
  test_no_neighbors();
  test_limit_max_out();
  test_invalid_vertex();
  printf("\tTest neighbors passed\n");

  test_is_path_clear();
  test_get_side_dir_30();
  test_find_best_move();
  // test_get_next_closest_objective();
  // test_all_objectives_visited();
  // test_shortest_path_no_wall();
  // test_shortest_path_with_wall();
  // test_player_on_objective();
  // test_multiple_objectives();
  // test_large_graph();
  // test_evaluation_functions();

  printf("\tTest first player strategy\n");

  return 0;
}
