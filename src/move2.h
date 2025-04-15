#include "move.h" 
#pragma once

//struct player (ajout last-position)
struct player_tt {
  enum player_color_t c;
  unsigned int walls;
  vertex_t position;
  vertex_t last_position; // pour déduire la direction
};


void index_to_axial(int index, int m, int *l, int *c);
int direction_axial(int dl, int dc);
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opp);
int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move);
void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move);
int availableMoves(struct move_t moves[], struct graph_t *graph, struct player_tt *p ,vertex_t opponent);
int path_to_objective_exists(struct graph_t *g, vertex_t start, const vertex_t *objectives, size_t nb_obj);
struct move_t* make_move_moove(enum player_color_t color, vertex_t dest) ;