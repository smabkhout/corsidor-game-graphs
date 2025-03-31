
#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
//enum graph_type_t type;
struct board_t *board ; 

char const* get_player_name()
{
  return "player1";
}


void initialize(unsigned int id, struct graph_t* graph) {

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives", id , graph-> num_vertices , graph->num_edges , graph->num_objectives) ; 
}



struct move_t play(const struct move_t previous_move) {
    struct move_t move;
    
    move.c = previous_move.c; 
    move.t = MOVE; 

    unsigned int new_pos = previous_move.m;  

    while (1) {
        new_pos = (new_pos + 1) % board->size;  

        
        move = create_move(previous_move.c, MOVE, new_pos, NULL);

        if (is_valid_move(&move, board->graph)) {
            printf("Player %d moves to vertex %u\n", move.c, move.m);
            break;  
        }
    }

    return move; 
}



void finalize(){
  board_free(board);
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