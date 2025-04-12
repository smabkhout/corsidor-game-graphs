#include "move.h" 


//struct player (ajout last-position)
struct player_tt {
  unsigned int walls;
  vertex_t position;
  vertex_t last_position; // pour déduire la direction
};
void index_to_axial(int index, int m, int *l, int *c);
int direction_axial(int dl, int dc);
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opp);
int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move);
void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move);
int  availableMoves(struct move_t* moves[], struct graph_t *graph, int id_ofplayer, struct move_t* previous_move);      
