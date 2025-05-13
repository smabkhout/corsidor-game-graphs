#include "board.h"
#include "graph.h"
#include "move2.h"
#include "player.h"
#include "strategies.h"
#include <gsl/gsl_spmatrix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define NO_VERTEX ((vertex_t)(-1))

#define MAX_COLOR player_id
#define MIN_COLOR (1 - player_id)
// enum graph_type_t type;
static struct board_t *board = NULL;
static unsigned int    player_id;
vertex_t               home_pos    = -1;
vertex_t               my_pos      = -1;
vertex_t               my_last_pos = -1;
vertex_t               opp_pos     = -1;
vertex_t               opp_las_pos = -1;
// static vertex_t previous_position;
// static int has_played = 0;
int      numberOfObjectives;
int     *visited_objectives     = NULL;
int     *visited_objectives_opp = NULL;
vertex_t home;
int      return_toHome;
int      m                                                         = 0;
int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin) = NULL;

const struct axial_t direec[7] = {
    {0, 0},   // No edge
    {1, -1},  // NW
    {1, 0},   // NE
    {0, 1},   // E
    {-1, 1},  // SE
    {-1, 0},  // SW
    {0, -1}   // W
};

char const *get_player_name() {
  srand(time(NULL));
  char *names[] = {"3aaazi", "rafiq"};
  return names[0];
}

void initialize(unsigned int id, struct graph_t *graph) {
  // Retrouver m
  // int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
  // Choix de la fonction selon le type
  resolve_graph_type_or_default(graph, &m, &in_hexagon);

  numberOfObjectives     = graph->num_objectives;
  visited_objectives     = malloc(sizeof(int) * numberOfObjectives);
  visited_objectives_opp = malloc(sizeof(int) * numberOfObjectives);
  return_toHome          = 0;
  for (int i = 0; i < numberOfObjectives; i++) {
    visited_objectives[i] = 0;
  }

  for (int i = 0; i < numberOfObjectives; i++) {
    visited_objectives_opp[i] = 0;
  }
  player_id = id;
  home      = graph->start[player_id];

  board = board_init();
  // board->graph = malloc(sizeof(struct graph_t));
  board->graph = graph;
  if (!board->graph) {
    fprintf(stderr, "Erreur allocation du graph\n");
    exit(EXIT_FAILURE);
  }

  // copy_graph(board->graph, graph);

  printf(
      "Player %d initialized on graph with %u vertices and %u edges , and "
      "with %u objectives\n",
      id, board->graph->num_vertices, board->graph->num_edges, board->graph->num_objectives);

  home        = board->graph->start[player_id];
  opp_pos     = board->graph->start[(player_id + 1) % NUM_PLAYERS];
  my_pos      = board->graph->start[player_id];
  my_last_pos = board->graph->start[player_id];
  opp_las_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];
}

int evaluate(struct game_state *state, int color) {
  //
  vertex_t **paths                       = malloc(sizeof(vertex_t *) * numberOfObjectives);
  int       *distances_to_objectives     = malloc(sizeof(int) * numberOfObjectives);
  vertex_t **opp_paths                   = malloc(sizeof(vertex_t *) * numberOfObjectives);
  int       *opp_distances_to_objectives = malloc(sizeof(int) * numberOfObjectives);

  int count = 0;
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives[i]) {
      paths[i]                   = NULL;
      distances_to_objectives[i] = -1;
      continue;
    }
    count++;
    paths[i]                   = malloc(board->graph->num_vertices * sizeof(vertex_t));
    distances_to_objectives[i] = shortest_path_length(
        board->graph, my_pos, board->graph->objectives[i], opp_pos, paths[i], my_last_pos);
  }
  int p_count = 0;
  // calculate all distances to non-visited objectives for the opponent
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives_opp[i]) {
      opp_paths[i]                   = NULL;
      opp_distances_to_objectives[i] = -1;
      continue;
    }
    p_count++;
    opp_paths[i]                   = malloc(board->graph->num_vertices * sizeof(vertex_t));
    opp_distances_to_objectives[i] = shortest_path_length(
        board->graph, opp_pos, board->graph->objectives[i], my_pos, opp_paths[i], opp_las_pos);
  }
  // find the closest objective for me
  int min_distance = INT_MAX;
  int obj_index;
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (!visited_objectives[i] && distances_to_objectives[i] < min_distance &&
        distances_to_objectives[i] != -1) {
      obj_index    = i;
      min_distance = distances_to_objectives[i];
    }
  }

  // find the closest objective for the opponent
  int min_distance_opp = INT_MAX;
  int obj_index_opp;
  for (int i = 0; i < numberOfObjectives; ++i) {
    // print all variables in if for debug

    if (!visited_objectives_opp[i] && opp_distances_to_objectives[i] < min_distance_opp &&
        opp_distances_to_objectives[i] != -1) {
      obj_index_opp    = i;
      min_distance_opp = opp_distances_to_objectives[i];
    }
  }
  free(paths);
  free(opp_paths);
  free(distances_to_objectives);
  free(opp_distances_to_objectives);

  return min_distance - min_distance_opp;
}

struct scored_move minmax(struct game_state *state, int depth, int color) {
  // Cas de base : profondeur 0 → évaluation
  if (depth == 0) {
    struct move_t move;
    move.t = MOVE;
    move.m = state->previous_moves[color].m;
    move.c = color;

    struct scored_move scored_move;
    scored_move.score = evaluate(state, color);
    scored_move.move  = move;

    return scored_move;
  }

  struct scored_move best_move;

  // Initialisation du meilleur score selon le joueur
  if (color == MAX_COLOR) {
    best_move.score = INT_MIN;
  } else {
    best_move.score = INT_MAX;
  }

  // Préparation du joueur courant
  struct move_t    possible_moves[128];
  struct player_tt p;
  p.position      = state->previous_moves[color].m;
  p.c             = color;
  p.last_position = state->previous_positions[color];

  // Génération des coups disponibles
  int nb_moves =
      availableMovess(possible_moves, state->graph, &p, state->previous_moves[1 - color].m);

  for (int i = 0; i < nb_moves; ++i) {
    struct move_t move = possible_moves[i];

    // Appliquer le coup → nouvel état
    struct game_state new_state = applyy_move(state, move);

    // Appel récursif minmax
    int score = minmax(&new_state, depth - 1, 1 - color).score;

    // Mise à jour du meilleur coup
    if ((color == MAX_COLOR && score > best_move.score) ||
        (color == MIN_COLOR && score < best_move.score)) {
      best_move.score = score;
      best_move.move  = move;
    }
  }

  return best_move;
}

struct move_t play(const struct move_t previous_move) {
  if (previous_move.t == MOVE && previous_move.c != player_id) {
    opp_las_pos = opp_pos;
    opp_pos     = previous_move.m;
  }
  if (previous_move.t == WALL && previous_move.c != player_id) {
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

  for (int i = 0; i < numberOfObjectives; i++) {
    if (my_pos == board->graph->objectives[i]) {
      visited_objectives[i] = 1;
    }
    if (opp_pos == board->graph->objectives[i]) {
      visited_objectives_opp[i] = 1;
    }
  }

  int all_objectives_are_visited = 1;
  for (int i = 0; i < numberOfObjectives; i++) {
    if (visited_objectives[i] == 0)
      all_objectives_are_visited = 0;
  }
  if (all_objectives_are_visited) {
    struct move_t move;
    vertex_t     *lile = malloc(board->graph->num_vertices * sizeof(vertex_t));
    lile[0]            = 0;
    lile[1]            = 0;
    printf("returning to home : %d\n", home);
    printf("position of the other player %d :\n", opp_pos);
    vertex_t objectif = home;
    vertex_t temp     = opp_pos;
    // cas particulier si l'adversaire est dans home quand le joueur essaie de revenir
    if (objectif == opp_pos) {                   // à change apres par place wall
      opp_pos = board->graph->num_vertices + 1;  // aller vers le sommet d'indice precedent
    }
    int result =
        shortest_path_length(board->graph, my_pos, objectif, opp_pos, lile, my_last_pos) + 1;
    if (result == 1 && objectif == temp) {
      opp_pos = temp;
      objectif -= 1;
      result = shortest_path_length(board->graph, my_pos, objectif, opp_pos, lile, my_last_pos) + 1;
    }
    printf("resultat : %d  ", result);
    for (int i = 0; i < result; i++) {
      printf(" %d;", lile[i]);
    }
    printf("\n");
    move.c      = player_id;
    move.t      = MOVE;
    move.m      = lile[1];
    my_last_pos = my_pos;
    my_pos      = move.m;
    if (move.m == objectif) {
      return_toHome = 1;
    }
    free(lile);

    return move;
  }
  struct scored_move best_move;
  struct game_state  state;
  state.graph                 = board->graph;
  state.previous_moves[0].m   = my_pos;
  state.previous_moves[1].m   = opp_pos;
  state.previous_positions[0] = my_last_pos;
  state.previous_positions[1] = opp_las_pos;
  best_move                   = minmax(&state, 4, player_id);
  struct move_t move          = best_move.move;
}

void finalize() {
  if (visited_objectives) {
    free(visited_objectives);
    free(visited_objectives_opp);
  }
  if (board) {
    board_free(board);
    board = NULL;
  }
}
