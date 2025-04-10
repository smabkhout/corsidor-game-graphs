#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

//enum graph_type_t type;
static struct graph_t *graph1= NULL ; 
static unsigned int player_id;
static vertex_t previous_position;
static int has_played = 0;


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
  graph1= malloc(sizeof(struct graph_t)) ; 
  if (!graph1){
    fprintf(stderr, "Erreur d'allocation\n");
    exit(EXIT_FAILURE);
  }

  copy_graph(graph1 , graph) ; 

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , graph1-> num_vertices , graph1->num_edges , graph1->num_objectives);

}


struct move_t play(const struct move_t previous_move) {
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
}



int is_connected1(struct graph_t *graph, vertex_t v1, vertex_t v2) {
    if (!graph || v1 >= graph->num_vertices || v2 >= graph->num_vertices) {
        return 0;  // Vérification de validité des indices
    }

    // Vérification de l'existence d'une arête entre v1 et v2
    if (gsl_spmatrix_uint_get(graph->t, v1, v2) > 0) {
        return 1;
    }

    return 0;
}


/*struct move_t play(const struct move_t previous_move) {
    struct move_t move;
    move.c = (previous_move.c+1)%2;
    move.t = MOVE;

    // Trouver une position voisine valide
    for (unsigned int i = 0; i < graph1->num_vertices; i++) {
        if (is_connected1(graph1, graph1->start[move.c], i)) {
            move.m = i;
            graph1->start[move.c] = 1 ; 
            printf("Player %d moves to vertex %u\n", move.c, move.m);
            return move;
        }
    }

    // Si aucun mouvement valide, on reste sur place
    move.m = graph1->start[move.c]  ;
    return move;
}*/



void finalize() {
    if (graph1) {  
        gsl_spmatrix_uint_free(graph1->t);  
        free(graph1->objectives);  
        free(graph1);  
        graph1 = NULL;  
    }
}


