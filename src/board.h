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
enum dir_t get_direction(vertex_t from, vertex_t to, struct graph_t* graph) ;
struct move_t make_move_move(enum player_color_t color, vertex_t dest);
int is_path_clear(struct graph_t* graph, vertex_t from, enum dir_t dir, int dist, vertex_t opponent_pos, vertex_t* result);
void get_side_dir_30(enum dir_t dir, enum dir_t* d1, enum dir_t* d2);
struct move_t find_best_move(struct graph_t* graph, vertex_t pos, vertex_t opponent, enum dir_t prev_dir, enum player_color_t color);
#endif // _CORS_BOARD_H_