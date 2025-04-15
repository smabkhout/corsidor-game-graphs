#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

//enum graph_type_t type;
static struct board_t *board = NULL ; 
//static unsigned int player_id;
//static vertex_t previous_position;
//static int has_played = 0;


char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[rand() % 2];
}



void initialize(unsigned int id, struct graph_t* graph) {
    board = board_init();
    // board->graph = malloc(sizeof(struct graph_t));
    board->graph = graph;
      if (!board->graph) {
          fprintf(stderr, "Erreur allocation du graph\n");
          exit(EXIT_FAILURE);
      }
  
      // copy_graph(board->graph, graph); 
  
    printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , board->graph-> num_vertices , board->graph->num_edges , board->graph->num_objectives);
  
  }




struct move_t play(const struct move_t previous_move) {
    struct move_t move;

    move.t = NO_TYPE;
    move.c = previous_move.c == NO_COLOR ? BLACK : (previous_move.c + 1) % 2;
    move.m = 0; 
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;

    printf("👻 Player %d plays a NO_TYPE move (mock behavior)\n", move.c);

    if (board != NULL) {
        add_move_to_board(board, move);
    } else {
        printf("Board non initialisé dans play()\n");
    }

    return move;
}


void finalize() {
    if (board) {
        board_free(board);
        board = NULL;
    }
}


