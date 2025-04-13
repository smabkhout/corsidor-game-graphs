#include "board.h"
#include "graph_functions.h"
#include <stdio.h>
#include <stdlib.h>

struct board_t *board_init() {
  struct board_t *board = malloc(sizeof(struct board_t));
  board->moves = malloc(sizeof(struct move_t));
  board->size_moves = 0;
  board->wall_count = 0;
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

void add_move_to_board(struct board_t *board, struct move_t move) {
  if (move.t == WALL) {
    board->wall_count++;
  }
  board->moves =
      realloc(board->moves, (board->size_moves + 1) * sizeof(struct move_t));
  board->moves[board->size_moves] = move;
  board->size_moves++;
}

/*void board_free(struct board_t* board){
    //graph_free(board->graph);
    free(board->moves);
    free(board);
}*/

void board_free(struct board_t *board) {
  if (board) {
    free(board->moves);
    if (board->graph) {
      puts("board->graph is freed");
      graph_free(board->graph);
    }
    free(board);
  }
  board = NULL;
}

// on doit voir comment on peut generer wall sans que graph soit en parametre
// pour l utiliser dans la fct play
void generate_wall(struct edge_t e[2], struct board_t *board) {
  (void)board;
  e[0].fr = axial_to_index(1, 0, 5);
  e[0].to = axial_to_index(0, 1, 5);
  e[1].to = axial_to_index(0, 1, 5);
  e[1].fr = axial_to_index(0, -1, 5);
}

int is_invalid(struct move_t move, struct board_t *board) {
  if (!board || !board->graph)
    return 1;

  struct graph_t *graph = board->graph;
  vertex_t my_pos = graph->start[move.c];
  vertex_t opp_pos = graph->start[(move.c + 1) % NUM_PLAYERS];

  if (move.t == MOVE) {
    if (move.m >= graph->num_vertices)
      return 1;
    if (move.m == opp_pos)
      return 1;
    if (gsl_spmatrix_uint_get(graph->t, my_pos, move.m) == 7)
      return 1;
    if (gsl_spmatrix_uint_get(graph->t, my_pos, move.m) == 0)
      return 1;

    return 0;
  }

  else if (move.t == WALL) {
    for (int i = 0; i < 2; i++) {
      if (move.e[i].fr >= graph->num_vertices ||
          move.e[i].to >= graph->num_vertices)
        return 1;
    }
    for (int i = 0; i < 2; i++) {
      if (gsl_spmatrix_uint_get(graph->t, move.e[i].fr, move.e[i].to) == 0)
        return 1;
    }
    if (move.e[0].fr != move.e[1].fr)
      return 1;

    enum dir_t d0 = gsl_spmatrix_uint_get(graph->t, move.e[0].fr, move.e[0].to);
    enum dir_t d1 = gsl_spmatrix_uint_get(graph->t, move.e[1].fr, move.e[1].to);

    if (abs((int)d0 - (int)d1) != 1 && abs((int)d0 - (int)d1) != 5)
      return 1;

    return 0;
  }

  return 1;
}
