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
  player_id              = id;
  home                   = graph->start[player_id];

  board = board_init();
  // board->graph = malloc(sizeof(struct graph_t));
  board->graph = graph;
  if (!board->graph) {
    fprintf(stderr, "Erreur allocation du graph\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < numberOfObjectives; i++) {
    int l = 0;
    int c = 0;
    index_to_axial(board->graph->objectives[i], m, &l, &c, graph->type);
    if (!in_hexagon(l, c, m, 0, 0)) {
      visited_objectives[i]     = 1;
      visited_objectives_opp[i] = 1;
    }
    visited_objectives[i] = 0;
  }

  for (int i = 0; i < numberOfObjectives; i++) {
    int l = 0;
    int c = 0;
    index_to_axial(board->graph->objectives[i], m, &l, &c, graph->type);
    if (!in_hexagon(l, c, m, 0, 0)) {
      visited_objectives[i]     = 1;
      visited_objectives_opp[i] = 1;
    }
    visited_objectives_opp[i] = 0;
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
    if (return_toHome) {
      struct move_t    availableMovees[128];
      struct player_tt p;
      p.position      = my_pos;
      p.last_position = my_last_pos;
      p.c             = player_id;
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
          my_pos      = availableMovees[i].m;
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
        shortest_path_astar(board->graph, my_pos, objectif, opp_pos, lile, my_last_pos) + 1;
    if (result == 1 && objectif == temp) {
      opp_pos = temp;
      objectif -= 1;
      result = shortest_path_astar(board->graph, my_pos, objectif, opp_pos, lile, my_last_pos) + 1;
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

  // calculate all distances to non-visited objectives
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
    paths[i] = malloc(board->graph->num_vertices * sizeof(vertex_t));
    for (vertex_t j = 0; j < board->graph->num_vertices; ++j) {
      paths[i][j] = NO_VERTEX;
    }
    distances_to_objectives[i] = shortest_path_astar(
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
    opp_paths[i] = malloc(board->graph->num_vertices * sizeof(vertex_t));
    for (vertex_t j = 0; j < board->graph->num_vertices; ++j) {
      opp_paths[i][j] = NO_VERTEX;
    }
    opp_distances_to_objectives[i] = shortest_path_astar(
        board->graph, opp_pos, board->graph->objectives[i], my_pos, opp_paths[i], opp_las_pos);
  }
  // find the closest objective for me
  int min_distance = INT_MAX;
  int obj_index    = -1;
  for (int i = 0; i < numberOfObjectives; ++i) {
    // printf("visited_objectives[%d] : %d \n", i, visited_objectives[i]);
    // printf("distances_to_objectives[%d] : %d \n", i, distances_to_objectives[i]);
    // printf("min_distance : %d \n", min_distance);
    if (!visited_objectives[i] && distances_to_objectives[i] < min_distance &&
        distances_to_objectives[i] != -1) {
      obj_index    = i;
      min_distance = distances_to_objectives[i];
    }
  }
  if (obj_index < 0) {
    min_distance = -1;
    printf("no reachable objective for the player, falling back to home\n");
  }

  // find the closest objective for the opponent
  int min_distance_opp = INT_MAX;
  int obj_index_opp    = -1;
  for (int i = 0; i < numberOfObjectives; ++i) {
    // print all variables in if for debug
    /*
        printf("visited_objectives_opp[%d] : %d \n", i, visited_objectives_opp[i]);
        printf("opp_distances_to_objectives[%d] : %d \n", i, opp_distances_to_objectives[i]);
        printf("min_distance_opp : %d \n", min_distance_opp);
        printf("min_distance : %d \n", min_distance);
    */
    if (!visited_objectives_opp[i] && opp_distances_to_objectives[i] < min_distance_opp &&
        opp_distances_to_objectives[i] != -1) {
      // printf("distances to objectives for the opponent %d : %d \n",
      // opp_distances_to_objectives[i],
      //     i);

      obj_index_opp    = i;
      min_distance_opp = opp_distances_to_objectives[i];
    }
  }

  if (obj_index_opp < 0) {
    min_distance_opp = -1;
    printf("no reachable objective for the opponent, falling back to home\n");
  }
  /*
  printf("my position : %d \n", my_pos);
  printf("opponent position : %d \n", opp_pos);

  puts("the distances me to objectives and opp to objectives :\n");
  printf("my distance to the nearest objective : %d \n", min_distance);
  printf("opp distance to the objective  : %d \n", min_distance_opp);
  */

  if (min_distance == -1) {
    if (min_distance_opp == -1) {
      printf("no reachable objective for both players, falling back to home\n");
      struct move_t move;
      move.t              = MOVE;
      move.c              = player_id;
      vertex_t *home_path = malloc(board->graph->num_vertices * sizeof(vertex_t));
      for (vertex_t j = 0; j < board->graph->num_vertices; ++j) {
        home_path[j] = NO_VERTEX;
      }
      int result = shortest_path_astar(board->graph, my_pos, home, opp_pos, home_path, my_last_pos);
      if (result == 1 && home == opp_pos) {
        opp_pos = board->graph->num_vertices + 1;  // aller vers le sommet d'indice precedent
        result  = shortest_path_astar(board->graph, my_pos, home, opp_pos, home_path, my_last_pos);
      }
      if (result == -1) {
        printf("I cannot go home, I will go next to it\n");

        int l_home;
        int c_home;
        index_to_axial(home - 1, m, &l_home, &c_home, board->graph->type);

        home   = (in_hexagon(l_home - 1, c_home - 1, m, 0, 0)) ? home - 1 : home + 1;
        result = shortest_path_astar(board->graph, my_pos, home, opp_pos, home_path, my_last_pos);
        move.m = home_path[1];
        move.t = MOVE;
        move.c = player_id;
        free(home_path);
        free(paths);
        free(distances_to_objectives);
        free(opp_paths);
        free(opp_distances_to_objectives);
        return move;
      }

      move.m      = home_path[1];
      my_last_pos = my_pos;
      my_pos      = home_path[1];
      free(home_path);
      free(paths);
      free(distances_to_objectives);
      free(opp_paths);
      free(opp_distances_to_objectives);
      return move;
    } else {
      puts("No reachable objective for me, but the opponent has one, I must block him");
      printf("the distance to the objective for the opponent is %d\n", min_distance_opp);
      printf("the distance to the objective for me is %d\n", min_distance);

      // place a wall to stop him
      struct move_t wall;
      wall.t = WALL;
      wall.c = player_id;

      // Convertir les index en coordonnées axiales
      // opponent pos
      int l0 = 0;
      int c0 = 0;
      // opponent next_pos (based on his shortest path)
      int l1 = 0;
      int c1 = 0;
      index_to_axial(opp_pos, m, &l0, &c0, board->graph->type);
      index_to_axial(opp_paths[obj_index_opp][1], m, &l1, &c1, board->graph->type);
      printf("opponent path : %d to %d\n", opp_pos, opp_paths[obj_index_opp][1]);
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
      // the first vertex to cut is (l0,c0) to (l1,c1)
      int l1_1 = l0 + direec[dir].l;
      int c1_1 = c0 + direec[dir].c;
      // the second one is either through the next direction or through the previous one
      printf("the direction is %d\n", dir);
      int next_direction = next_dir(dir);
      printf("the next direction is %d\n", next_direction);
      int l2_2 = l0 + direec[next_direction].l;
      int c2_2 = c0 + direec[next_direction].c;
      if (!in_hexagon(l2_2, c2_2, m, 0, 0)) {
        // on simule une fonction prev_dir
        next_direction = (dir == 0) ? 0 : (dir == 6) ? FIRST_DIR : (dir + 1);
        l2_2           = l0 + direec[next_direction].l;
        c2_2           = c0 + direec[next_direction].c;
      }

      vertex_t to1 = axial_to_index(l1_1, c1_1, m, board->graph->type);
      vertex_t to2 = axial_to_index(l2_2, c2_2, m, board->graph->type);
      wall.e[0].fr = opp_pos;
      wall.e[1].fr = opp_pos;
      wall.e[0].to = to1;
      wall.e[1].to = to2;
      wall.m       = my_pos;
      printf("I want to put a wall form %d to (%d, %d) and (%d, %d)\n", opp_pos, l1_1, c1_1, l2_2,
             c2_2);

      /*
      // just for testing before putting a wall
      struct player_tt *p = malloc(sizeof(struct player_tt));
      p->walls            = 50;
      printf("The wall is being built from %d to : %d and %d.\n", opp_pos, to1, to2);
      assert(valid_wall(board->graph, p, wall));
      free(p);
      */

      // now we need to check whether there is a path to all objectives or not in order not to cut
      // the road for the opponent
      int blocked = 0;

      unsigned int *temp      = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[0].fr, wall.e[0].to);
      unsigned int  old_temp  = *temp;
      *temp                   = 7;
      unsigned int *temp1     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[1].fr, wall.e[1].to);
      unsigned int  old_temp1 = *temp1;
      *temp1                  = 7;
      unsigned int *temp3     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[0].to, wall.e[0].fr);
      unsigned int  old_temp3 = *temp3;
      *temp3                  = 7;
      unsigned int *temp4     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[1].to, wall.e[1].fr);
      unsigned int  old_temp4 = *temp4;
      *temp4                  = 7;

      for (int i = 0; i < numberOfObjectives; ++i) {
        vertex_t *opp_path = malloc(board->graph->num_vertices * sizeof(vertex_t));
        int       distance = shortest_path_astar(board->graph, opp_pos, board->graph->objectives[i],
                                                 my_pos, opp_path, opp_las_pos);
        free(opp_path);
        if (distance == -1) {
          blocked = 1;
          break;
        }
      }

      if (!blocked) {
        for (int i = 0; i < numberOfObjectives; ++i) {
          if (visited_objectives[i])
            continue;
          free(paths[i]);
        }
        for (int i = 0; i < numberOfObjectives; ++i) {
          if (visited_objectives_opp[i])
            continue;
          free(opp_paths[i]);
        }
        free(paths);
        free(distances_to_objectives);
        free(opp_paths);
        free(opp_distances_to_objectives);
        return wall;
      } else {  // we return the graph to its original state and continue
        *temp  = old_temp;
        *temp1 = old_temp1;
        *temp3 = old_temp3;
        *temp4 = old_temp4;
      }
    }
  }

  if (min_distance_opp < min_distance) {
    // printf("the distance to the objective for the opponent is %d\n", min_distance_opp);
    // printf("the distance to the objective for me is %d\n", min_distance);

    // place a wall to stop him
    struct move_t wall;
    wall.t = WALL;
    wall.c = player_id;

    // Convertir les index en coordonnées axiales
    // opponent pos
    int l0 = 0;
    int c0 = 0;
    // opponent next_pos (based on his shortest path)
    int l1 = 0;
    int c1 = 0;
    index_to_axial(opp_pos, m, &l0, &c0, board->graph->type);
    index_to_axial(opp_paths[obj_index_opp][1], m, &l1, &c1, board->graph->type);
    printf("opponent path : %d to %d\n", opp_pos, opp_paths[obj_index_opp][1]);
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
    // the first vertex to cut is (l0,c0) to (l1,c1)
    int l1_1 = l0 + direec[dir].l;
    int c1_1 = c0 + direec[dir].c;
    // the second one is either through the next direction or through the previous one
    printf("the direction is %d\n", dir);
    int next_direction = next_dir(dir);
    printf("the next direction is %d\n", next_direction);
    int l2_2 = l0 + direec[next_direction].l;
    int c2_2 = c0 + direec[next_direction].c;
    if (!in_hexagon(l2_2, c2_2, m, 0, 0)) {
      // on simule une fonction prev_dir
      next_direction = (dir == 0) ? 0 : (dir == 6) ? FIRST_DIR : (dir + 1);
      l2_2           = l0 + direec[next_direction].l;
      c2_2           = c0 + direec[next_direction].c;
    }

    vertex_t to1 = axial_to_index(l1_1, c1_1, m, board->graph->type);
    vertex_t to2 = axial_to_index(l2_2, c2_2, m, board->graph->type);
    wall.e[0].fr = opp_pos;
    wall.e[1].fr = opp_pos;
    wall.e[0].to = to1;
    wall.e[1].to = to2;
    wall.m       = my_pos;
    printf("I want to put a wall form %d to (%d, %d) and (%d, %d)\n", opp_pos, l1_1, c1_1, l2_2,
           c2_2);

    /*
    // just for testing before putting a wall
    struct player_tt *p = malloc(sizeof(struct player_tt));
    p->walls            = 50;
    printf("The wall is being built from %d to : %d and %d.\n", opp_pos, to1, to2);
    assert(valid_wall(board->graph, p, wall));
    free(p);
    */

    // now we need to check whether there is a path to all objectives or not in order not to cut
    // the road for the opponent
    int blocked = 0;

    unsigned int *temp      = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[0].fr, wall.e[0].to);
    unsigned int  old_temp  = *temp;
    *temp                   = 7;
    unsigned int *temp1     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[1].fr, wall.e[1].to);
    unsigned int  old_temp1 = *temp1;
    *temp1                  = 7;
    unsigned int *temp3     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[0].to, wall.e[0].fr);
    unsigned int  old_temp3 = *temp3;
    *temp3                  = 7;
    unsigned int *temp4     = gsl_spmatrix_uint_ptr(board->graph->t, wall.e[1].to, wall.e[1].fr);
    unsigned int  old_temp4 = *temp4;
    *temp4                  = 7;

    for (int i = 0; i < numberOfObjectives; ++i) {
      vertex_t *opp_path = malloc(board->graph->num_vertices * sizeof(vertex_t));
      int distance = shortest_path_astar(board->graph, opp_pos, board->graph->objectives[i], my_pos,
                                         opp_path, opp_las_pos);
      free(opp_path);
      if (distance == -1) {
        blocked = 1;
        break;
      }
    }

    if (!blocked) {
      for (int i = 0; i < numberOfObjectives; ++i) {
        if (visited_objectives[i])
          continue;
        free(paths[i]);
      }
      for (int i = 0; i < numberOfObjectives; ++i) {
        if (visited_objectives_opp[i])
          continue;
        free(opp_paths[i]);
      }
      free(paths);
      free(distances_to_objectives);
      free(opp_paths);
      free(opp_distances_to_objectives);
      return wall;
    } else {  // we return the graph to its original state and continue
      *temp  = old_temp;
      *temp1 = old_temp1;
      *temp3 = old_temp3;
      *temp4 = old_temp4;
    }
  }

  struct move_t move;
  move.c = player_id;
  move.t = MOVE;
  printf("objective index : %d \n", obj_index);
  move.m = paths[obj_index][1];

  printf("Player %d found this path using A* to objective %d :\n", player_id,
         board->graph->objectives[obj_index]);
  for (vertex_t v = 0; paths[obj_index][v] != (unsigned int)-1; ++v) {
    printf("%d, ", paths[obj_index][v]);
  }
  printf("\n");

  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives[i])
      continue;
    free(paths[i]);
  }
  for (int i = 0; i < numberOfObjectives; ++i) {
    if (visited_objectives_opp[i])
      continue;
    free(opp_paths[i]);
  }
  free(paths);
  free(distances_to_objectives);
  free(opp_paths);
  free(opp_distances_to_objectives);

  /*
  for (int i = 0 ; i<num_ofObjective ; i++){
      int t = shortest_path_astar(board->graph, my_pos,
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
    struct move_t    availableMovees[128];
    struct player_tt p;
    p.position      = my_pos;
    p.last_position = my_last_pos;
    p.c             = player_id;
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
        my_pos      = availableMovees[i].m;
        return availableMovees[i];
      }
    }
  }

  my_last_pos = my_pos;
  my_pos      = move.m;
  return move;
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
