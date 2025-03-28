#include "move.h"



struct move_t create_move(enum player_color_t color, enum move_type_t type, vertex_t vertex, struct edge_t edges[2]) ; 

int can_place_wall(struct graph_t * graph, struct edge_t e[2]);

int is_empty_position(const struct graph_t* graph, vertex_t n ); 

int can_move(struct graph_t * graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player);

int is_valid_move(const struct move_t* move, const struct graph_t* graph) ; 

void apply_move(struct move_t* move, struct graph_t* graph) ;

