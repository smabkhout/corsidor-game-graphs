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

// enum graph_type_t type;
static struct board_t *board = NULL;
static unsigned int player_id;
vertex_t home_pos = -1;
vertex_t my_pos = -1;
vertex_t my_last_pos = -1;
vertex_t opp_pos = -1;
vertex_t opp_las_pos = -1;
// static vertex_t previous_position;
// static int has_played = 0;
int numberOfObjectives;
int *visited_objectives = NULL;
vertex_t home;
int return_toHome;

const struct axial_t direec[7] = {
    {0, 0},  // No edge
    {1, -1}, // NW
    {1, 0},  // NE
    {0, 1},  // E
    {-1, 1}, // SE
    {-1,0}, // SW
    {0,-1}  // W
};

char const *get_player_name() {
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[0];
}

void initialize(unsigned int id, struct graph_t *graph) {
  numberOfObjectives = graph->num_objectives;
  visited_objectives = malloc(sizeof(int) * numberOfObjectives);
  for (int i = 0; i < numberOfObjectives; i++) {
    visited_objectives[i] = 0;
  }
  player_id = id;
  home = graph->start[player_id];

  board = board_init();
  // board->graph = malloc(sizeof(struct graph_t));
  board->graph = graph;
  if (!board->graph) {
    fprintf(stderr, "Erreur allocation du graph\n");
    exit(EXIT_FAILURE);
  }

  // copy_graph(board->graph, graph);

  printf("Player %d initialized on graph with %u vertices and %u edges , and "
         "with %u objectives\n",
         id, board->graph->num_vertices, board->graph->num_edges,
         board->graph->num_objectives);

  home = board->graph->start[player_id];
  opp_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];
  my_pos = board->graph->start[player_id];
  my_last_pos = board->graph->start[player_id];
  opp_las_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];
}

struct move_t play(const struct move_t previous_move) {

  if (previous_move.t == MOVE && previous_move.c != player_id) {
    opp_las_pos = opp_pos;
    opp_pos = previous_move.m;
  }
  if (previous_move.t == WALL && previous_move.c != player_id) {

    unsigned int *temp = gsl_spmatrix_uint_ptr(
        board->graph->t, previous_move.e[0].fr, previous_move.e[0].to);
    *temp = 7;
    unsigned int *temp1 = gsl_spmatrix_uint_ptr(
        board->graph->t, previous_move.e[1].fr, previous_move.e[1].to);
    *temp1 = 7;
    unsigned int *temp3 = gsl_spmatrix_uint_ptr(
        board->graph->t, previous_move.e[0].to, previous_move.e[0].fr);
    *temp3 = 7;
    unsigned int *temp4 = gsl_spmatrix_uint_ptr(
        board->graph->t, previous_move.e[1].to, previous_move.e[1].fr);
    *temp4 = 7;
  }

  for (int i = 0; i < numberOfObjectives; i++) {
    if (my_pos == board->graph->objectives[i]) {
      visited_objectives[i] = 1;
    }
  }

  int all_objectives_are_visited = 1;
  for (int i = 0; i < numberOfObjectives; i++) {
    if (visited_objectives[i] == 0)
      all_objectives_are_visited = 0;
  }
  if (all_objectives_are_visited) {
    if (return_toHome) {
      struct move_t availableMovees[128];
      struct player_tt p;
      p.position = my_pos;
      p.last_position = my_last_pos;
      p.c = player_id;
      // id of player

      int result = availableMoves(availableMovees, board->graph, &p, opp_pos);
      /*
      for (int i = 0 ; i<result ; i++){
        printf(" moves disponible %d \n " ,availableMovees[i].m  ) ;
      }
      */
      // puts("\n") ;
      for (int i = 0; i < result; i++) {
        printf(" moves disponible %d \n ", availableMovees[i].m);
        availableMovees[i].t = MOVE;
        availableMovees[i].c = player_id;
        if (availableMovees[i].m != my_pos) {
          my_last_pos = my_pos;
          my_pos = availableMovees[i].m;
          return availableMovees[i];
        }
      }
      printf("\n");
      // availableMovees[0].t = MOVE ;
      // availableMovees[0].c = player_id ;
      // my_last_pos = availableMovees[0].m ;
      // return availableMovees[0];
    }
    struct move_t move;
    vertex_t *lile = malloc(board->graph->num_vertices * sizeof(vertex_t));
    lile[0] = 0;
    lile[1] = 0;
    printf("returning to home : %d\n", home);
    printf("position of the other player %d :\n", opp_pos);
    vertex_t objectif = home;
    int result = shortest_path_length(board->graph, my_pos, objectif, opp_pos,
                                      lile, my_last_pos) +
                 1;
    printf("resultat %d  ", result);
    for (int i = 0; i < result; i++) {
      printf(" %d;", lile[i]);
    }
    printf("\n");
    move.c = player_id;
    move.t = MOVE;
    move.m = lile[1];
    my_last_pos = my_pos;
    my_pos = move.m;
    if (move.m == objectif) {
      return_toHome = 1;
    }
    free(lile);

    return move;
  }

  // calculate all distances to non-visited objectives
  vertex_t **paths = malloc(sizeof(vertex_t *) * numberOfObjectives);
  int *distances_to_objectives = malloc(sizeof(int) * numberOfObjectives);
  vertex_t **opp_paths = malloc(sizeof(vertex_t *) * numberOfObjectives);
  int *opp_distances_to_objectives = malloc(sizeof(int) * numberOfObjectives);

  int count = 0;
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives[i]) {
      paths[i] = NULL;
      distances_to_objectives[i] = -1;
      continue;
    }
    count++;
    paths[i] = malloc(board->graph->num_vertices * sizeof(vertex_t));
    distances_to_objectives[i] =
        shortest_path_length(board->graph, my_pos, board->graph->objectives[i],
                             opp_pos, paths[i], my_last_pos);
  }
  int p_count = 0;
  // calculate all distances to non-visited objectives for the opponent
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives[i]) {
      opp_paths[i] = NULL;
      opp_distances_to_objectives[i] = -1;
      continue;
    }
    p_count++;
    opp_paths[i] = malloc(board->graph->num_vertices * sizeof(vertex_t));
    opp_distances_to_objectives[i] =
        shortest_path_length(board->graph, opp_pos, board->graph->objectives[i],
                             my_pos, opp_paths[i], opp_las_pos);
  }
  // find the closest objective for me
  int min_distance = INT_MAX;
  int obj_index;
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (!visited_objectives[i] && distances_to_objectives[i] < min_distance &&
        distances_to_objectives[i] != -1) {
      obj_index = i;
      min_distance = distances_to_objectives[i];
    }
  }

  // find the closest objective for the opponent
  int min_distance_opp = INT_MAX;
  int obj_index_opp;
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (!visited_objectives[i] &&
        opp_distances_to_objectives[i] < min_distance_opp &&
        opp_distances_to_objectives[i] != -1) {
      obj_index_opp = i;
      min_distance_opp = opp_distances_to_objectives[i];
    }
  }
    puts("the distances me to objectifes and opp to objectives :\n");
    printf("me : %d \n" , min_distance);
    printf("opp : %d \n" , min_distance_opp);
  if (min_distance_opp < min_distance) {
    // place a wall to stop him
    struct move_t wall;
    wall.t = WALL;
    wall.c = player_id;

    // Convertir les index en coordonnées axiales
    int l0 = 0;
    int c0 = 0;
    int l1 = 0;
    int c1 = 0;
    index_to_axial(opp_pos, 5, &l0, &c0);
    index_to_axial(opp_paths[obj_index_opp][1], 5, &l1,
                   &c1);

    int dl_prev = l1 - l0;
    int dc_prev = c1 - c0;

    // Normalisation du vecteur direction afin qu'il soit reconnu par
    // direction_axial par exemple dir (3, 0) devient (1, 0)
    int max_abs = fmax(abs(dl_prev), abs(dc_prev));
    if (max_abs != 0) {
      dl_prev /= max_abs;
      dc_prev /= max_abs;
    }

    int dir = direction_axial(dl_prev, dc_prev);
    int l1_1 = l0 + direec[dir].l;
    int c1_1 = c0 + direec[dir].c;
    int l2_2 = l0 + direec[(dir + 1) % 6].l;
    int c2_2 = c0 + direec[(dir + 1) % 6].c;
    vertex_t to1 = axial_to_index(l1_1, c1_1, 5);
    vertex_t to2 = axial_to_index(l2_2, c2_2,5);
    wall.e[0].fr = opp_pos;
    wall.e[1].fr = opp_pos;
    wall.e[0].to = to1;
    wall.e[1].to = to2;
    wall.m = my_pos;
    return wall;
  }
  else{  
  struct move_t move;
  move.c = player_id;
  move.t = MOVE;
  move.m = paths[obj_index][1];

  printf("Player %d found this path using dijkstra :\n", player_id);
  for (vertex_t v = 0; paths[obj_index][v] != (unsigned int)-1; ++v) {
    printf("%d, ", paths[obj_index][v]);
  }
  printf("\n");

  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives[i])
      continue;
    free(paths[i]);
  }
  free(paths);
  free(distances_to_objectives);
  free(opp_paths);
  free(opp_distances_to_objectives);

  /*
  for (int i = 0 ; i<num_ofObjective ; i++){
      int t = shortest_path_length(board->graph, my_pos,
  board->graph->objectives[0], opp_pos, path, my_last_pos); if (t<taille &&
  visited_objectives[i]==0 ){ taille = t ; start = i ;
      }
  }
  */
  int length = min_distance;
  if (length == -1 || length == INT_MAX) {
    /*puts("No valid path to an objective");
    move.t = NO_TYPE;
    return move;*/
    struct move_t availableMovees[128];
    struct player_tt p;
    p.position = my_pos;
    p.last_position = my_last_pos;
    p.c = player_id;
    // id of player

    int result = availableMoves(availableMovees, board->graph, &p, opp_pos);
    /*
    for (int i = 0 ; i<result ; i++){
      printf(" moves disponible %d \n " ,availableMovees[i].m  ) ;
    }
    */
    // puts("\n") ;
    for (int i = 0; i < result; i++) {
      printf(" moves disponible %d \n ", availableMovees[i].m);
      availableMovees[i].t = MOVE;
      availableMovees[i].c = player_id;
      if (availableMovees[i].m != my_pos) {
        my_last_pos = my_pos;
        my_pos = availableMovees[i].m;
        return availableMovees[i];
      }
    }
  }

  my_last_pos = my_pos;
  my_pos = move.m;
  return move;
  }
}

void finalize() {
  if (visited_objectives) {
    free(visited_objectives);
  }
  if (board) {
    board_free(board);
    board = NULL;
  }
}
