#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#define NO_VERTEX ((vertex_t)(-1))

//enum graph_type_t type;
static struct board_t *board = NULL ; 
static unsigned int player_id;
//static vertex_t previous_position;
//static int has_played = 0;


char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[1];
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

int get_neighbors(struct graph_t* graph, vertex_t v, vertex_t* out, int max_out) {
    int count = 0;
    for (vertex_t i = 0; i < graph->num_vertices && count < max_out; i++) {
        if (gsl_spmatrix_uint_get(graph->t, v, i) != 0) {
            out[count++] = i;
        }
    }
    return count;
    }
struct move_t make_move_no_type() {
    struct move_t move;
    move.t = NO_TYPE;
    move.c = NO_COLOR;
    move.m = 0;
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;
    return move;
}


struct move_t play(const struct move_t previous_move) {
    static int steps = 0;
    static enum dir_t dir = W; // Par défaut vers West
    vertex_t my_pos = board->graph->start[player_id];
    vertex_t opp_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }

    vertex_t neighbors[8];
    int count = get_neighbors(board->graph, my_pos, neighbors, 8);

    for (int i = 0; i < count; i++) {
        vertex_t to = neighbors[i];
        enum dir_t d = gsl_spmatrix_uint_get(board->graph->t, my_pos, to);
        if (d == dir && to != opp_pos && steps < 3) {
            steps++;
            board->graph->start[player_id] = to;
            return make_move_move(player_id, to);
        }
    }

    enum dir_t left = (dir + 5) % 6;
    enum dir_t right = (dir + 1) % 6;
    for (int i = 0; i < count; i++) {
        vertex_t to = neighbors[i];
        enum dir_t d = gsl_spmatrix_uint_get(board->graph->t, my_pos, to);
        if ((d == left || d == right) && to != opp_pos) {
            dir = d;  
            steps = 1;
            board->graph->start[player_id] = to;
            return make_move_move(player_id, to);
        }
    }

    return make_move_no_type();
}




/*struct move_t play(const struct move_t previous_move) {
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
}*/


void finalize() {
    if (board) {
        board_free(board);
        board = NULL;
    }
}


