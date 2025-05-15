#include "graph.h"

#include "move2.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

typedef unsigned int vertex_t;

struct distance_node {
  vertex_t vertex;
  int      distance;
  bool     visited;
  int      num_moves;
};

struct game_state {
  struct graph_t *graph;
  struct move_t   previous_moves[2];  // dernier coup pour chaque joueur
  vertex_t        previous_positions[2];
};

struct scored_move {
  int           score;
  struct move_t move;
};

vertex_t          min_distance_vertex(struct distance_node *nodes, size_t num_vertices);
int               shortest_path_length(struct graph_t *g, vertex_t start, vertex_t objective,
                                       vertex_t opponent_pos, vertex_t *path, vertex_t last_pos);
int               shortest_path_astar(struct graph_t *g, vertex_t start, vertex_t objective,
                                      vertex_t opponent_pos, vertex_t *path, vertex_t last_pos);
double            heuristic(vertex_t a, vertex_t b, int m, int type);
struct game_state applyy_move(const struct game_state *state, struct move_t move);
void test_AstarVSdIJKSTRA();
// int evaluate(struct game_state *state, int color);

// struct scored_move minmax(struct game_state *state, int depth, int color);