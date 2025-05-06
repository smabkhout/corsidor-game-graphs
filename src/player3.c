#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>


static struct board_t *board = NULL; 
static int obj_visited[5];
static int index_objective ;
static enum player_color_t my_color;

char const* get_player_name()
{
  srand(time(NULL));
  char *name = "dina";
  return name;
}


void initialize(unsigned int id, struct graph_t* graph) {
  board = board_init();
  board->graph = graph;
  board->current_positions[id]=graph->start[id];
    if (!board->graph) {
        fprintf(stderr, "Erreur allocation du graph\n");
        exit(EXIT_FAILURE);
    }
    my_color = id;
    index_objective=0;
  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , board->graph-> num_vertices , board->graph->num_edges , board->graph->num_objectives);
  printf("🧭 Objectifs déclarés :\n");
  for (unsigned int i = 0; i < board->graph->num_objectives; i++) {
      printf("  [%d] vertex = %d\n", i, board->graph->objectives[i]);
  }
  
  for(int i=0;i<5;i++){
    obj_visited[i]=-1;

  }
}



struct move_t play(const struct move_t previous_move) {
    (void)(previous_move);
    int best_order[5];
    TSP(board->graph,best_order,obj_visited);   
    struct move_t move ;
    move.t = MOVE;
    move.c = my_color;
    vertex_t pos_player=board->current_positions[move.c];
    vertex_t pos_opponent = board->current_positions[(my_color + 1) % NUM_PLAYERS];
    printf("la couleur du joueur %d\n",move.c);
    printf("la position du joueur %d\n ",pos_player);
    if (gsl_spmatrix_uint_get(board->graph->t, pos_player, pos_opponent) > 0) {
        enum dir_t dir_to_opponent = gsl_spmatrix_uint_get(board->graph->t, pos_player, pos_opponent);

        for (vertex_t i = 0; i < board->graph->num_vertices; ++i) {
            if (gsl_spmatrix_uint_get(board->graph->t, pos_opponent, i) == dir_to_opponent && i != pos_player && i != pos_opponent) {
                printf("🤸 Player %d jumps over the opponent to vertex %d\n", my_color, i);
                move.m = i;
                move.e[0].fr = move.e[0].to = 0;
                move.e[1].fr = move.e[1].to = 0;
                board->current_positions[my_color] = i;
                add_move_to_board(board, move);
                return move;
            }
        }
    }
    vertex_t target = board->graph->objectives[best_order[0]];
    if((unsigned int)index_objective==board->graph->num_objectives){
        target= board->graph->start[my_color];
    }
    int d[board->graph->num_vertices];
    int prev[board->graph->num_vertices];
    int next[board->graph->num_vertices];
    dijistra ( board->graph, pos_player, target , d, prev, next);
    printf("📏 Distance to target (%d): %d\n", target, d[target]);
    printf("la position de l objective %d\n",board->graph->objectives[0]);
    int d_enemy[board->graph->num_vertices];
    int prev_enemy[board->graph->num_vertices];
    int next_enemy[board->graph->num_vertices];
    dijistra(board->graph, pos_opponent, target, d_enemy, prev_enemy, next_enemy);
    move.m = next[pos_player]; 
    printf(" le sommet destinataire %d\n", move.m);
    board->current_positions[move.c]=move.m;
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;

    printf("👻 Player %d plays  moves \n", move.c);
    board->current_positions[my_color] = move.m;
    if (move.m == target) {
        printf("🎯 Objective %d reached!\n", index_objective);
        obj_visited[index_objective]=best_order[0];
        index_objective++;
        // Tous les objectifs atteints ? Retourner à la case de départ

    }
    if (d_enemy[target] < d[target]) {
        struct move_t wall_move = try_place_wall(board->graph, pos_opponent, next_enemy[pos_opponent], my_color);
        if (wall_move.t == WALL) {
            add_move_to_board(board, wall_move);
            return wall_move;
        }
    }
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
