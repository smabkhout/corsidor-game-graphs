#include <stdio.h>
#include <stdlib.h>
#include "board.h"


struct board_t* board_init(){
    struct board_t *board=malloc(sizeof(struct board_t));
    board->moves=malloc(sizeof(struct move_t));
    board->size=0;
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
    /*if (move.t == WALL ){
        wall_count ++;

    }*/
    board->moves=realloc(board->moves,(board->size+1)*sizeof(struct move_t));
    board->moves[board->size]=move ;
    board->size++;
}