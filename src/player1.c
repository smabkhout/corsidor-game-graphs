#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

//enum graph_type_t type;
static struct board_t *board = NULL;  
/*static unsigned int player_id;
static vertex_t previous_position;
static int has_played = 0;*/


char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[rand() % 2];
}


void copy_graph(struct graph_t* dest, const struct graph_t* src) {
    // Copier les champs simples
    dest->type = src->type;
    dest->num_vertices = src->num_vertices;
    dest->num_edges = src->num_edges;
    dest->num_objectives = src->num_objectives;
    
    memcpy(dest->start, src->start, sizeof(vertex_t) * NUM_PLAYERS); // Copier start[]

    // Copier la matrice creuse (sparse matrix)
    dest->t = gsl_spmatrix_uint_alloc(src->num_vertices, src->num_vertices);
    gsl_spmatrix_uint* csr = gsl_spmatrix_uint_compress(dest->t, GSL_SPMATRIX_CSR);
    gsl_spmatrix_uint_free(dest->t);
    dest->t = csr;
    if (!dest->t) {
        fprintf(stderr, "Erreur allocation mémoire pour t\n");
        exit(1);
    }
    gsl_spmatrix_uint_memcpy(dest->t, src->t);  // Copie de la matrice creuse

    // Copier les objectifs (tableau dynamique)
    dest->objectives = malloc(src->num_objectives * sizeof(vertex_t));
    if (!dest->objectives) {
        fprintf(stderr, "Erreur allocation mémoire pour objectives\n");
        exit(1);
    }
    memcpy(dest->objectives, src->objectives, src->num_objectives * sizeof(vertex_t));  // Copie du tableau
}


void initialize(unsigned int id, struct graph_t* graph) {
  board = board_init();
  board->graph = malloc(sizeof(struct graph_t));
    if (!board->graph) {
        fprintf(stderr, "Erreur allocation du graph\n");
        exit(EXIT_FAILURE);
    }

    copy_graph(board->graph, graph); 

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , board->graph-> num_vertices , board->graph->num_edges , board->graph->num_objectives);

}


/*struct move_t play(const struct move_t previous_move) {
    vertex_t my_pos = graph1->start[player_id];
    vertex_t opp_pos = graph1->start[(player_id + 1) % 2];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }

    enum dir_t prev_dir = NO_EDGE;
    if (has_played) {
        prev_dir = get_direction(previous_position, my_pos, graph1);
    } else {
        prev_dir = 3;
    }

    struct move_t move = find_best_move(graph1, my_pos, opp_pos, prev_dir, player_id);

    if (gsl_spmatrix_uint_get(graph1->t, my_pos, opp_pos) > 0) {
        for (vertex_t jump = 0; jump < graph1->num_vertices; jump++) {
            if (gsl_spmatrix_uint_get(graph1->t, opp_pos, jump) > 0 &&
                jump != my_pos && jump != opp_pos) {
                move = make_move_move(player_id, jump);
                break;
            }
        }
    }
    if (move.t == MOVE) {
        previous_position = my_pos;         
        graph1->start[player_id] = move.m;  
        has_played = 1;                     
    }
    return move;
}*/
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


/*void finalize() {
    if (graph1) {  
        gsl_spmatrix_uint_free(graph1->t);  
        free(graph1->objectives);  
        free(graph1);  
        graph1 = NULL;  
    }
}
*/

void finalize() {
    if (board) {
        board_free(board);
        board = NULL;
    }
}




