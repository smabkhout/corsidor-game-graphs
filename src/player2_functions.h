#ifndef _CORS_PLAYER2_FUNCTIONS_H_
#define _CORS_PLAYER2_FUNCTIONS_H_

int is_wall_ahead(struct player_t* player);

void move_back_wall_ahead(struct player_t *player,struct graph_t * graph);

void make_wall_otherplayer_nearer(struct player_t*player1 , struct player_t*player2, struct graph_t* graph);

int id_next_position(struct player_t *player);
#endif //_CORS_PLAYER2_FUNCTIONS_H_
