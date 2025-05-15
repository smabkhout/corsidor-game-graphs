#include "graph.h"
#include "player.h"
#include "board.h"
#include "strategie3.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#define MAX_OBJ 20
static struct board_t     *board = NULL;
static int                 obj_visited[MAX_OBJ];
static int                 index_objective;
static enum player_color_t my_color;
static vertex_t            start_player;

char const *get_player_name() {
  srand(time(NULL));
  char *name = "dina";
  return name;
}

void initialize(unsigned int id, struct graph_t *graph) {
  board                        = board_init();
  board->graph                 = graph;
  board->current_positions[id] = graph->start[id];
  start_player                 = graph->start[id];
  if (!board->graph) {
    fprintf(stderr, "Erreur allocation du graph\n");
    exit(EXIT_FAILURE);
  }
  my_color        = id;
  index_objective = 0;
  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n",
         id, board->graph->num_vertices, board->graph->num_edges, board->graph->num_objectives);
  printf("🧭 Objectifs déclarés :\n");
  for (unsigned int i = 0; i < board->graph->num_objectives; i++) {
    printf("  [%d] vertex = %d\n", i, board->graph->objectives[i]);
  }

  for (int i = 0; i < 5; i++) {
    obj_visited[i] = -1;
  }
}

struct move_t play(const struct move_t previous_move) {
  if (previous_move.t == MOVE) {
    board->current_positions[previous_move.c] = previous_move.m;
  }
  if (previous_move.t == WALL && previous_move.c != my_color) {
    unsigned int *temp =
        gsl_spmatrix_uint_ptr(board->graph->t, previous_move.e[0].fr, previous_move.e[0].to);
    *temp = 7;
    unsigned int *temp1 =
        gsl_spmatrix_uint_ptr(board->graph->t, previous_move.e[1].fr, previous_move.e[1].to);
    *temp1 = 7;
    unsigned int *temp3 =
        gsl_spmatrix_uint_ptr(board->graph->t, previous_move.e[0].to, previous_move.e[0].fr);
    *temp3 = 7;
    unsigned int *temp4 =
        gsl_spmatrix_uint_ptr(board->graph->t, previous_move.e[1].to, previous_move.e[1].fr);
    *temp4 = 7;
  }

  int           best_order[5];
  struct move_t move;
  move.t                = MOVE;
  move.c                = my_color;
  vertex_t pos_player   = board->current_positions[move.c];
  vertex_t pos_opponent = board->current_positions[(my_color + 1) % NUM_PLAYERS];
  TSP(board->graph, best_order, obj_visited, pos_player);
  vertex_t target = ((unsigned int)index_objective == board->graph->num_objectives)
                        ? start_player
                        : board->graph->objectives[best_order[0]];

  int d[board->graph->num_vertices];
  int prev[board->graph->num_vertices];
  int next[board->graph->num_vertices];
  dijistra(board->graph, pos_player, target, d, prev, next, pos_opponent);

  if (next[pos_player] == -1) {
    printf("⚠️ Dijkstra a échoué, aucun chemin trouvé depuis %d vers %d !\n", pos_player, target);

    move =
        find_best_move(board->graph, pos_player, pos_opponent,
                       gsl_spmatrix_uint_get(board->graph->t, start_player, pos_player), my_color);
    if (move.t != NO_TYPE && move.m != pos_player) {
      board->current_positions[my_color] = move.m;
      add_move_to_board(board, move);
      return move;
    }
    return move;
  }

  if ((unsigned int)next[pos_player] == pos_opponent) {
    enum dir_t dir_to_opponent = gsl_spmatrix_uint_get(board->graph->t, pos_player, pos_opponent);
    for (vertex_t candidate = 0; candidate < board->graph->num_vertices; ++candidate) {
      if (gsl_spmatrix_uint_get(board->graph->t, pos_opponent, candidate) == dir_to_opponent &&
          candidate != pos_opponent && candidate != pos_player &&
          gsl_spmatrix_uint_get(board->graph->t, pos_opponent, candidate) != WALL_DIR) {
        printf("🤸 Saut utile au-dessus de l’adversaire vers %d\n", candidate);
        move.m                             = candidate;
        board->current_positions[my_color] = move.m;
        move.e[0].fr = move.e[0].to = 0;
        move.e[1].fr = move.e[1].to = 0;
        add_move_to_board(board, move);
        return move;
      }
    }
  }

  move.m = next[pos_player];
  if (move.m == pos_player) {
    move.t = NO_TYPE;
    return move;
  }

  int d_enemy[board->graph->num_vertices];
  int prev_enemy[board->graph->num_vertices];
  int next_enemy[board->graph->num_vertices];
  dijistra(board->graph, pos_opponent, target, d_enemy, prev_enemy, next_enemy, pos_player);

  printf("📏 Distance to target (%d): %d\n", target, d[target]);
  printf("🎯 Prochain sommet: %d\n", move.m);

  board->current_positions[my_color] = move.m;
  // marquage d'objectif
  for (unsigned int i = 0; i < board->graph->num_objectives; i++) {
    if (obj_visited[i] == -1 && board->current_positions[my_color] == board->graph->objectives[i]) {
      obj_visited[i] = i;
      index_objective++;
      printf("🎯 Objectif %d (vertex %d) visité ! index_objective=%d\n", i,
             board->graph->objectives[i], index_objective);
      break;
    }
  }

  move.e[0].fr = move.e[0].to = 0;
  move.e[1].fr = move.e[1].to = 0;

  if ((d_enemy[target] < d[target]) && (next_enemy[pos_opponent] != -1) &&
      ((unsigned int)board->wall_count <= (board->graph->num_edges) / 10)) {
    struct move_t fall_back =
        find_best_move(board->graph, pos_player, pos_opponent,
                       gsl_spmatrix_uint_get(board->graph->t, start_player, pos_player), my_color);
    struct move_t wall_move =
        try_place_wall(board->graph, pos_opponent, next_enemy[pos_opponent], fall_back);
    if (wall_move.t == WALL) {
      add_move_to_board(board, wall_move);
      return wall_move;
    }
  }

  add_move_to_board(board, move);
  return move;
}

void finalize() {
  if (board) {
    board_free(board);
    board = NULL;
  }
}