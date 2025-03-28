#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 8 


struct board_t {
    struct move_t* moves;
    enum player_color_t cells[BOARD_SIZE][BOARD_SIZE]; 
   // int walls[BOARD_SIZE][BOARD_SIZE];
    int wall_count;
    struct graph_t* graph;
    int size;
};


void board_init() {
    struct board_t *board=malloc(sizeof(struct board_t));
    board->moves=malloc(sizeof(struct move_t));
    board->size=BOARD_SIZE;
    for (int i = 0; i < board->size; i++) {
        for (int j = 0; j < board->size; j++) {
            board->cells[i][j] = NO_COLOR; 
        }
    }
}

void display_board(struct board_t *board) {
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
}