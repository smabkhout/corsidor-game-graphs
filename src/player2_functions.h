#ifndef _CORS_PLAYER2_FUNCTIONS_H_
#define _CORS_PLAYER2_FUNCTIONS_H_

#include "move.h"
#include "graph.h"

int is_wall_ahead( vertex_t p, struct graph_t * graph);
int can_move(struct graph_t * graph, vertex_t a, vertex_t b);
int can_place_wall(struct graph_t * graph, struct edge_t e[2]);
struct move_t* best_move(struct player_t* player, struct graph_t * graph);
enum move_type_t choose_move_wall(struct player_t* player, struct graph_t * graph);
void make_wall_otherplayer_nearer(struct player_t*player1 , struct player_t*player2, struct graph_t* graph);

int id_next_position(struct player_t *player);
#endif //_CORS_PLAYER2_FUNCTIONS_H_
