#ifndef _CORS_BOARD_H_
#define _CORS_BOARD_H_
#include "move.h"
#include "graph.h"



#define BOARD_SIZE 8 

struct board_t {
    struct move_t* moves;
   // int walls[BOARD_SIZE][BOARD_SIZE];
    int wall_count;
    struct graph_t* graph;
    int size;
};

struct board_t* board_init();
//void display_board(struct board_t *board);
void add_move_to_board(struct board_t* board, struct move_t move);

#endif // _CORS_BOARD_H_