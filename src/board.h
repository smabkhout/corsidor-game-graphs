#ifndef _CORS_BOARD_H_
#define _CORS_BOARD_H_
#include "graph_functions.h"
#include "move_functions.h"




struct board_t {
    struct move_t* moves;
    int wall_count;
    struct graph_t* graph;
    int size_moves;
    vertex_t current_positions[2] ; 
};

struct board_t* board_init();
//void display_board(struct board_t *board);
void add_move_to_board(struct board_t* board, struct move_t move);
void board_free(struct board_t* board);
// Fonction pour générer des murs dans le jeu.
// Cette fonction remplit le tableau d'arêtes 'e' avec des positions aléatoires.
void generate_wall(struct edge_t e[2], struct board_t * board);

int is_invalid(struct move_t move, struct board_t* board);
#endif // _CORS_BOARD_H_