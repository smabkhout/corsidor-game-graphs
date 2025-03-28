
#include <gsl/gsl_spmatrix.h>
#include "graph.h"

#include "player.h"
#include "move_functions.h"
#include "move.h"
#include "board.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>



void initialize(unsigned int player_id, struct graph_t* graph){
  //
}


char const* get_player_name()
{
  return "palyer1";
}




struct move_t play(const struct move_t previous_move) {
    struct move_t move;
    
    move.c = previous_move.c; 
    move.t = MOVE; 

    unsigned int new_pos = previous_move.m;  

    while (1) {
        new_pos = (new_pos + 1) % BOARD_SIZE;  

        
        move = create_move(previous_move.c, MOVE, new_pos, NULL);

        if (is_valid_move(&move, board->graph)) {
            printf("Player %d moves to vertex %u\n", move.c, move.m);
            break;  
        }
    }

    return move; 
}



void finalize()
{

}


int main() {
    unsigned int n = 4;  // Number of vertices in the graph
    struct graph_t* graph = malloc(sizeof(struct graph_t));
    initialize(graph , n , CYCLIC) ; 

    
    // Initialisation de la position initiale (par exemple à la position 0)
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
}