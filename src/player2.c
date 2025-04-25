#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#include "move2.h"
#include "strategies.h"
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
    vertex_t my_pos = board->graph->start[player_id];
    vertex_t opp_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }
    // si board ->size_moves <4 ; fait un move aleatoire 
    if (board->size_moves < 4) {
        while(1){
        struct move_t move;
        move.t = MOVE;
        move.c = player_id;
        move.m = rand() % board->graph->num_vertices;
        struct player_tt p;
        p.position = my_pos;
        p.last_position = board->moves[board->size_moves - 4].m;
        p.c = player_id;
        if (valid_move(board->graph, &p, move.m, my_pos)) {
            return move;
        }
        
        }
    }else {
    struct move_t *moove = malloc(sizeof(struct move_t));
    struct move_t availbel[128];
    struct player_tt p;
    p.position = my_pos;
    p.last_position = board->moves[board->size_moves - 4].m;
    p.c = player_id;

    int count = availableMoves(availbel, board->graph, &p, opp_pos);
    int score=0 ;
    for (int i = 0; i < count; i++) {
        struct move_t move;
        move = availbel[i];
        vertex_t *path = malloc(board->graph->num_vertices * sizeof(vertex_t));
        if (shortest_path_length(board->graph, move.m ,board->graph->objectives[0],opp_pos , path) > score) {
            score = shortest_path_length(board->graph, move.m ,board->graph->objectives[0],opp_pos , path);
            *moove = move;
             ;
            }
        free(path);
        }
        moove->c = player_id;
        return *moove;
    }

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


