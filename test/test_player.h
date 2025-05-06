
#include "strategie3.h"
#include "graph_functions.h"
#include "player.h"
#include "move.h"
#include "board.h"
#include "graph.h"

void test_neighbors_basic();
void test_max_out_zero();
void test_no_neighbors();
void test_limit_max_out();
void test_invalid_vertex();
// void test_play();
void           free_graph(struct graph_t* g);
struct graph_t init_graph_example();
void           test_is_path_clear();
void           test_get_side_dir_30();
void           test_find_best_move();
// void test_get_next_closest_objective();
// void test_all_objectives_visited();
