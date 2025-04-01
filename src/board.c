#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "graph_functions.h"

struct board_t* board_init(){
    struct board_t *board=malloc(sizeof(struct board_t));
    board->moves=malloc(sizeof(struct move_t));
    board->size=0;
    board->wall_count=0;
    return board;
}

/*void display_board(struct board_t *board) {
    for (int i = 0; i < board->size; i++) {
        for (int j = 0; j < board->size; j++) {
            char symbol;
            switch (board->cells[i][j]) {
                case BLACK: symbol = 'B'; break;
                case WHITE: symbol = 'W'; break;
                default: symbol = '.'; break; 
            }
            printf("%c ", symbol);
        }
        printf("\n");
    }
}*/


void add_move_to_board(struct board_t* board, struct move_t move){
    if (move.t == WALL ){
        board->wall_count++;

    }
    board->moves=realloc(board->moves,(board->size+1)*sizeof(struct move_t));
    board->moves[board->size]=move;
    board->size++;
}

void board_free(struct board_t* board){
    free(board->moves);
    free(board);
}


//on doit voir comment on peut generer wall sans que graph soit en parametre pour l utiliser dans la fct play
void generate_wall(struct edge_t e[2], struct board_t *board) {
    e[0].fr = axial_to_index(1, 0, 5) ;
    e[0].to=  axial_to_index(0,1,5);
    e[1].to = axial_to_index(0,1,5);
    e[1].fr = axial_to_index(0,-1,5);
}