
/*
#include "player.h"
#include "board.h"
#include "move_functions.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>

//enum graph_type_t type;
//struct board_t *board ; 

struct graph_t graph1 ; 

char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"Adam", "Khaoula", "Rafiq"};
  return names[rand() % 3];
}


void initialize(unsigned int id, struct graph_t* graph) {
  if (id > 1 || graph == NULL) {
    return; 
  }
  graph->start[id] = 27;

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , graph-> num_vertices , graph->num_edges , graph->num_objectives);
}

struct move_t play(const struct move_t previous_move){




























    struct move_t move;
    
    move.c = (previous_move.c+1)%2 ; 
    srand(time(NULL));
    enum move_type_t mv = 2;
    move.t = mv; 

    unsigned int new_pos = previous_move.m;
    //struct edge_t e[2] ;
    //generate_wall(e); 

    while (1) {
        new_pos = (new_pos + 1) % 61;  

        
        move = create_move(previous_move.c, MOVE, new_pos, NULL);

       // if (is_valid_move(&move, board->graph)) {
         //   printf("Player %d moves to vertex %u\n", move.c, move.m);
            break;  
        //}
    }

    return move;

}


void finalize(){
  //board_free(board);
}


/*int main() {
    unsigned int n = 4;  // Number of vertices in the graph
    struct graph_t* graph = malloc(sizeof(struct graph_t));
    initialize_graph(graph , n , CYCLIC) ; 

    
    struct move_t previous_move;
    previous_move.c = 1;  // Exemple de couleur de joueur
    previous_move.t = MOVE;
    previous_move.m = 0;  // Position initiale du joueur

    // Le graphe pour l'exemple
    struct board_t* board = malloc(sizeof(struct board_t));
    board->graph = graph ; 

    // Appel de la fonction play avec un mouvement initial
    struct move_t new_move = play(previous_move);

    // Affichage du résultat
    printf("New move: Player %d moves to vertex %u\n", new_move.c, new_move.m);

    return 0;
}*/




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
    puts("erreur dans l'allocation du graph pour player1 ") ; 
  }

  copy_graph(graph1 , graph) ; 

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , graph1-> num_vertices , graph1->num_edges , graph1->num_objectives);

}


/*
struct move_t play(const struct move_t previous_move) {
    struct move_t move;
    
    move.c = previous_move.c; 
    srand(time(NULL));
    enum move_type_t mv = 2;
    move.t = mv; 

    unsigned int new_pos = previous_move.m;
    //struct edge_t e[2] ;
    //generate_wall(e); 

    while (1) {
        new_pos = (new_pos + 1) % 61 ;  

        
        move = create_move(previous_move.c, MOVE, new_pos, NULL);

        //if (is_valid_move(&move, board->graph)) {
          //  printf("Player %d moves to vertex %u\n", move.c, move.m);
            break;  
        //}
    }

    return move;

}*/



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


struct move_t play(const struct move_t previous_move) {
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
}



void finalize() {
    if (graph1) {  
        gsl_spmatrix_uint_free(graph1->t);  
        free(graph1->objectives);  
        free(graph1);  
        graph1 = NULL;  
    }
}


/*int main() {
    unsigned int n = 4;  // Number of vertices in the graph
    struct graph_t* graph = malloc(sizeof(struct graph_t));
    initialize_graph(graph , n , CYCLIC) ; 

    
    struct move_t previous_move;
    previous_move.c = 1;  // Exemple de couleur de joueur
    previous_move.t = MOVE;
    previous_move.m = 0;  // Position initiale du joueur

    // Le graphe pour l'exemple
    struct board_t* board = malloc(sizeof(struct board_t));
    board->graph = graph ; 

    // Appel de la fonction play avec un mouvement initial
    struct move_t new_move = play(previous_move);

    // Affichage du résultat
    printf("New move: Player %d moves to vertex %u\n", new_move.c, new_move.m);

    return 0;
}*/

