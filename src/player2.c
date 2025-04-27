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
vertex_t my_pos = -1;
vertex_t my_last_pos = -1;
vertex_t opp_pos = -1;
//static vertex_t previous_position;
//static int has_played = 0;
int numberOfObjectives ; 
int* visited_objectives = NULL; 
vertex_t home ; 
int return_toHome ; 
char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[1];
}




void initialize(unsigned int id, struct graph_t* graph) {
    numberOfObjectives = graph->num_objectives ;
    visited_objectives = malloc(sizeof(int) * numberOfObjectives);
    for(int i = 0 ; i<numberOfObjectives ; i++){
        visited_objectives[i] = 0 ; 
    }
    player_id = id;
    home = graph->start[player_id] ; 
    
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

    int num_ofObjective = numberOfObjectives;
    if (my_pos == (unsigned int)-1)
        my_pos = board->graph->start[player_id];
    if (opp_pos == (unsigned int)-1)
        opp_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }
   
    /*
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
    } else { */
        /*
    struct move_t *moove = malloc(sizeof(struct move_t));
    struct move_t availbel[128];
    struct player_tt p;
    p.position = my_pos;
    p.last_position = board->moves[board->size_moves - 4].m;
    p.c = player_id;
    //id of player 

    int count = availableMoves(availbel, board->graph, &p, opp_pos);
    int score=0 ;
    
*/  
    for (int i = 0 ; i<num_ofObjective ; i++){
        if (my_pos == board->graph->objectives[i]){
            visited_objectives[i] = 1 ;
        }
    }


    int all_objectives_are_visited = 1 ; 
    for (int i = 0 ; i< num_ofObjective ; i++){
        if (visited_objectives[i] == 0 )
            all_objectives_are_visited = 0 ; 
    }

    if (all_objectives_are_visited){
        //if (return_toHome){
        struct move_t availableMovees[128] ; 
        struct player_tt p;
        p.position = my_pos;
        p.last_position = my_last_pos ; 
        p.c = player_id;
        //id of player 

        availableMoves(availableMovees, board->graph, &p, opp_pos);
        availableMovees[0].t = MOVE ; 
        availableMovees[0].c = player_id ; 
        my_last_pos = availableMovees[0].m ; 
        return availableMovees[0] ; 
       // }
        /*
        struct move_t move;
        vertex_t *lile = malloc(board->graph->num_vertices * sizeof(vertex_t));
        lile[0] = 0;
        lile[1] = 0;
        printf("returning to home : %d \n" , home ) ; 
        vertex_t objectif = home;  
        int result = shortest_path_length(board->graph, my_pos, objectif, opp_pos, lile, my_last_pos) +1 ;
        printf("resultat %d  " , result) ; 
        for (int i =0 ; i<result ; i++){
            printf(" %d;" , lile[i]); 
        }
        printf("\n") ;
        move.c = player_id;
        move.t = MOVE;
        move.m = lile[1];
        my_last_pos = move.m ; 
        if (move.m == home){
            return_toHome =1 ; 
        }
        free(lile ) ; 

        return move  ; 
        */

    }

/*
    struct move_t move;
    vertex_t *path = malloc(board->graph->num_vertices * sizeof(vertex_t));
    path[0] = 0;
    path[1] = 0;
     int start ; 
     int taille = 100000;
*/

    // calculate all distances to non-visited objectives
    vertex_t** paths = malloc(sizeof(vertex_t*)*numberOfObjectives);
    int* distances_to_objectives = malloc(sizeof(int)*numberOfObjectives);    
    for (int i = 0; i<numberOfObjectives; ++i) {
        if (visited_objectives[i]) {
            paths[i] = NULL;
            distances_to_objectives[i] = -1;
            continue;
        }
        paths[i] = malloc(board->graph->num_vertices * sizeof(vertex_t));
        distances_to_objectives[i] = shortest_path_length(board->graph, my_pos, board->graph->objectives[i], opp_pos, paths[i], my_last_pos);
    }


    // find the closest objective
    int min_distance = INT_MAX;
    int obj_index;
    for (int i = 0; i<numberOfObjectives; ++i) {
        if (!visited_objectives[i] && distances_to_objectives[i] < min_distance) {
            obj_index = i;
            min_distance = distances_to_objectives[i];
        }
    }
    
    struct move_t move;
    move.c = player_id;
    move.t = MOVE;
    printf("The objectives are %d and %d\n", board->graph->objectives[0], board->graph->objectives[1]);
    printf("obj_index is %d\n", obj_index);
    printf("%d\n", paths[obj_index][0]);
    move.m = paths[obj_index][1];

    printf("Player %d found this path using dijkstra :\n", player_id);
    for (vertex_t v = 0; paths[obj_index][v] != (unsigned int)-1; ++v) {
      printf("%d, ", paths[obj_index][v]);
    }
    printf("\n");
        
    for (int i = 0; i<numberOfObjectives; ++i) {
        if (visited_objectives[i])
            continue;
        free(paths[i]);
    }
    free(paths);

    /*
    for (int i = 0 ; i<num_ofObjective ; i++){
        int t = shortest_path_length(board->graph, my_pos, board->graph->objectives[0], opp_pos, path, my_last_pos);
        if (t<taille && visited_objectives[i]==0 ){
            taille = t ; 
            start = i ; 
        }
    }
    */
    int length = min_distance;
    if (length == -1) {
        puts("No valid path to an objective");
        move.t = NO_TYPE;
        return move;
    } /*else if (!length) {
        puts("Player is already in objective");
        move.t = NO_TYPE;
        free(path);
        return move;
    }*/
    else {
        my_last_pos = my_pos;
        my_pos = move.m;
        return move;
        
    }
    /*
    for (int i = 0; i < count; i++) {
        struct move_t move;
        move = availbel[i];
        if ( length > score) {
            score = length;
            *moove = move;
            }
        free(path);
        }
        moove->c = player_id;
        return *moove;
        */
    }
        

    // }
    
    






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
    if (visited_objectives != 0) {
        free(visited_objectives);
    }
    if (board) {
        board_free(board);
        board = NULL;
    }
}


