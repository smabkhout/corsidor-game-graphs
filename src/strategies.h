#include "graph.h"
#include "move.h"
#include "move2.h"
#include "player.h"
#include "board.h"
#include "graph_functions.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

typedef unsigned int vertex_t;

struct distance_node {
  vertex_t vertex;
  int distance;
  bool visited;
};



vertex_t min_distance_vertex(struct distance_node *nodes, size_t num_vertices) ; 
int shortest_path_length(struct graph_t *g, vertex_t start, vertex_t objective,
                         vertex_t opponent_pos, vertex_t *path) ;   