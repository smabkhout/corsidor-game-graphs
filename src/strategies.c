#include "strategies.h"
#include <gsl/gsl_spblas.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

// Fonction pour appliquer un mouvement à l'état du jeu

struct game_state applyy_move(const struct game_state *state, struct move_t move) {
  struct game_state new_state          = *state;
  new_state.previous_positions[move.c] = state->previous_moves[move.c].m;
  new_state.previous_moves[move.c]     = move;
  return new_state;
}

// Structure pour stocker les distances dans Dijkstra

// Fonction utilitaire pour trouver le sommet non visité avec la distance
// minimale
vertex_t min_distance_vertex(struct distance_node *nodes, size_t num_vertices) {
  int      min_dist   = INT_MAX;
  int      min_moves  = INT_MAX;
  vertex_t min_vertex = 0;

  for (vertex_t v = 0; v < num_vertices; ++v) {
    if (!nodes[v].visited) {
      if (nodes[v].distance < min_dist ||
          (nodes[v].distance == min_dist && nodes[v].num_moves < min_moves)) {
        min_dist   = nodes[v].distance;
        min_moves  = nodes[v].num_moves;
        min_vertex = v;
      }
    }
  }

  return min_vertex;
}

// Fonction principale Dijkstra
int shortest_path_length(struct graph_t *g, vertex_t start, vertex_t objective,
                         vertex_t opponent_pos, vertex_t *path, vertex_t last_pos) {
  if (start == objective)
    return 0;
  int path_length = 0;

  int               m          = 0;
  in_hexagon_func_t in_hexagon = NULL;

  resolve_graph_type_or_default(g, &m, &in_hexagon);

  vertex_t n = 3 * (m * m) - 3 * m + 1;
  // Initialisation des nœuds
  struct distance_node *nodes = malloc(n * sizeof(struct distance_node));
  if (!nodes)
    return -1;

  // Nouveau tableau pour reconstruire le chemin
  vertex_t *prev = malloc(n * sizeof(vertex_t));
  if (!prev) {
    free(nodes);
    return -1;
  }
  for (vertex_t i = 0; i < n; ++i) {
    prev[i] = -1;
  }

  for (vertex_t v = 0; v < n; ++v) {
    int l;
    int c;
    index_to_axial(v, m, &l, &c, g->type);
    // if (!in_hexagon(l, c, m, 0, 0)) {
    //   nodes[v].vertex = -1;
    //  continue;
    //}
    nodes[v].vertex    = v;
    nodes[v].distance  = INT_MAX;
    nodes[v].visited   = false;
    nodes[v].num_moves = 0;
  }
  nodes[start].distance = 0;
  path[path_length]     = start;

  // Algorithme de Dijkstra
  for (size_t i = 0; i < n; ++i) {
    vertex_t u = min_distance_vertex(nodes, n);
    if (u == objective || nodes[u].distance == INT_MAX) {
      break;
    }

    nodes[u].visited    = true;
    vertex_t *neighbors = malloc(sizeof(vertex_t) * 6 * 3);
    int       count     = 0;
    // Parcourir tous les voisins de u
    for (vertex_t v = 0; v < n; ++v) {
      if (nodes[v].vertex == (unsigned int)-1) {
        continue;
      }
      struct player_tt p;
      p.position      = u;
      p.c             = 0;
      p.last_position = (u == start) ? last_pos : prev[u];
      // Vérifier si l'arête existe et n'est pas un mur
      if (valid_move(g, &p, v, opponent_pos)) {
        neighbors[count++] = v;
      }
    }
    if (count == 0) {
      free(nodes);
      return 0;
    }

    // s'assurer que dijkstra maintenant va prioriser les sauts de distance 3 si
    // c'est possible
    for (int wanted_jump = 3; wanted_jump >= 1; --wanted_jump) {
      for (int i = 0; i < count; ++i) {
        vertex_t v = neighbors[i];
        if (nodes[v].vertex == (unsigned int)-1) {
          continue;
        }

        struct player_tt p;
        p.position        = u;
        p.c               = 0;
        p.last_position   = (u == start) ? last_pos : prev[u];
        int jump_distance = valid_move(g, &p, v, opponent_pos);

        if (jump_distance == wanted_jump) {  // 🎯 on regarde seulement ceux du bon saut
          if (nodes[u].distance + 1 < nodes[v].distance) {
            nodes[v].distance = nodes[u].distance + 1;
            ++nodes[v].num_moves;  // distance en termes de sauts
            prev[v] = u;
          }
        }
      }
    }
    free(neighbors);
  }

  int result = nodes[objective].distance;

  if (result != INT_MAX) {
    vertex_t current = objective;
    path_length      = 0;

    // Reconstruction du chemin à l'envers
    while (current != (unsigned int)-1) {
      path[path_length++] = current;
      current             = prev[current];
    }

    // Inverser le chemin
    for (int i = 0; i < path_length / 2; ++i) {
      vertex_t tmp              = path[i];
      path[i]                   = path[path_length - 1 - i];
      path[path_length - 1 - i] = tmp;
    }

    path[path_length] = -1;  // terminaison
  } else {
    path[0]     = -1;  // pas de chemin trouvé
    path_length = 0;
  }

  free(nodes);
  free(prev);

  return (result == INT_MAX) ? -1 : result;
}

double heuristic(vertex_t a, vertex_t b, int m, int type) {
  int la, ca, lb, cb;
  index_to_axial(a, m, &la, &ca, type);
  index_to_axial(b, m, &lb, &cb, type);

  int dx = ca - cb;
  int dy = la - lb;
  return sqrt(dx * dx + dy * dy);
}

vertex_t min_fscore_vertex(struct distance_node *nodes, double *f_score, size_t num_vertices) {
  double   min_f     = DBL_MAX;
  int      min_moves = INT_MAX;
  vertex_t min_v     = 0;

  for (vertex_t v = 0; v < num_vertices; ++v) {
    if (!nodes[v].visited) {
      if (f_score[v] < min_f || (f_score[v] == min_f && nodes[v].num_moves < min_moves)) {
        min_f     = f_score[v];
        min_v     = v;
        min_moves = nodes[v].num_moves;
      }
    }
  }
  return min_v;
}

int shortest_path_astar(struct graph_t *g, vertex_t start, vertex_t objective,
                        vertex_t opponent_pos, vertex_t *path, vertex_t last_pos) {
  if (start == objective)
    return 0;

  int               path_length = 0;
  int               m           = 0;
  in_hexagon_func_t in_hexagon  = NULL;
  resolve_graph_type_or_default(g, &m, &in_hexagon);
  // vertex_t n = 3 * (m * m) - 3 * m + 1;
  vertex_t n = g->num_vertices;

  struct distance_node *nodes   = malloc(n * sizeof(struct distance_node));
  double               *f_score = malloc(n * sizeof(double));
  vertex_t             *prev    = malloc(n * sizeof(vertex_t));
  if (!nodes || !f_score || !prev) {
    free(nodes);
    free(f_score);
    free(prev);
    return -1;
  }

  for (vertex_t v = 0; v < n; ++v) {
    nodes[v].vertex    = v;
    nodes[v].distance  = INT_MAX;
    nodes[v].visited   = false;
    nodes[v].num_moves = 0;
    f_score[v]         = DBL_MAX;
    prev[v]            = -1;
  }

  nodes[start].distance = 0;
  f_score[start]        = heuristic(start, objective, m, g->type);
  path[path_length]     = start;

  for (size_t i = 0; i < n; ++i) {
    vertex_t u = min_fscore_vertex(nodes, f_score, n);
    if (u == objective || nodes[u].distance == INT_MAX)
      break;

    nodes[u].visited = true;

    vertex_t *neighbors = malloc(sizeof(vertex_t) * 6 * 3);
    int       count     = 0;
    for (vertex_t v = 0; v < n; ++v) {
      if (nodes[v].vertex == (unsigned int)-1)
        continue;

      struct player_tt p = {
          .position = u, .c = 0, .last_position = (u == start) ? last_pos : prev[u]};

      if (valid_move(g, &p, v, opponent_pos)) {
        neighbors[count++] = v;
      }
    }

    if (count == 0) {
      puts("NO VALID NEIGHBORS, OUT !!");
      free(nodes);
      free(f_score);
      free(prev);
      return 0;
    }

    for (int wanted_jump = 3; wanted_jump >= 1; --wanted_jump) {
      for (int i = 0; i < count; ++i) {
        vertex_t v = neighbors[i];
        if (nodes[v].vertex == (unsigned int)-1)
          continue;

        struct player_tt p = {
            .position = u, .c = 0, .last_position = (u == start) ? last_pos : prev[u]};
        int jump_distance = valid_move(g, &p, v, opponent_pos);

        if (jump_distance == wanted_jump) {
          int tentative_g = nodes[u].distance + 1;
          if (tentative_g < nodes[v].distance) {
            nodes[v].distance  = tentative_g;
            nodes[v].num_moves = nodes[u].num_moves + 1;
            prev[v]            = u;
            f_score[v]         = tentative_g + heuristic(v, objective, m, g->type);
          }
        }
      }
    }
    free(neighbors);
  }

  int result = nodes[objective].distance;
  printf("The result found to obj %d is %d\n", objective, result);

  if (result != INT_MAX) {
    vertex_t current = objective;
    path_length      = 0;

    while (current != (unsigned int)-1) {
      path[path_length++] = current;
      current             = prev[current];
    }

    for (int i = 0; i < path_length / 2; ++i) {
      vertex_t tmp              = path[i];
      path[i]                   = path[path_length - 1 - i];
      path[path_length - 1 - i] = tmp;
    }
    path[path_length] = -1;
  } else {
    path[0]     = -1;
    path_length = 0;
  }

  printf("The path found to objective %d is : ", objective);
  for (int i = 0; i < path_length; ++i) {
    printf("%d ", path[i]);
  }
  printf("\n");

  free(nodes);
  free(f_score);
  free(prev);

  return (result == INT_MAX) ? -1 : result;
}
// test A*
/*
void test_astar() {
  struct graph_t *g = createGraph(5, TRIANGULAR);
  g->num_objectives = 1;
  g->objectives     = malloc(sizeof(vertex_t) * 1);
  g->objectives[0]  = 29;

  vertex_t start = 1;
  vertex_t objective = g->objectives[0];
  vertex_t opponent_pos = 54;
  vertex_t last_pos = 0;

  vertex_t path[128];
  int result = shortest_path_astar(g, start, objective, opponent_pos, path, last_pos);
  print_hex_grid(g);
  printf("Shortest path length: %d\n", result);
  printf("Path: ");
  for (int i = 0; i < result +1; ++i) {
    printf("%d ", path[i]);
  }
  printf("\n");

  free(g->objectives);
}*/

int shortest_path_player(vertex_t start, vertex_t objective, vertex_t opponent_pos,
                         struct graph_t *g, vertex_t last_pos) {
  vertex_t path[128];
  return shortest_path_length(g, start, objective, opponent_pos, path, last_pos);
}

int evaluate(struct game_state *state, int color) {
  return shortest_path_player(state->previous_moves[color].m, state->graph->objectives[0],
                              state->previous_moves[1 - color].m, state->graph,
                              state->previous_positions[color]) -
         shortest_path_player(state->previous_moves[1 - color].m, state->graph->objectives[0],
                              state->previous_moves[color].m, state->graph,
                              state->previous_positions[1 - color]);
  ;
}

/*int is_game_over(struct game_state *state) {

  return 0;  // Jeu non terminé
}*/
// application de l'algorithme MinMax
#include <limits.h>  // Pour INT_MAX et INT_MIN

#define MAX_COLOR 0
#define MIN_COLOR 1

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

// test MINMAX
/*
void test_minmax() {
  struct game_state state;
  struct graph_t   *g = createGraph(5, TRIANGULAR);
  g->num_objectives = 1;
  g->objectives     = malloc(sizeof(vertex_t) * 1);
  g->objectives[0]  = 29;
  state.graph       = g;
  state.previous_moves[0].m = 1;
  g->start[0] = 1;
  g->start[1] = 54;
  state.previous_moves[1].m = 54;
  state.previous_positions[0] = 0;
  state.previous_positions[1] = 60;
  state.previous_moves[0].c = 0;
  state.previous_moves[1].c = 1;
  printf("WHITE: %d\n", WHITE);
  printf("BLACK: %d\n", BLACK);

  struct scored_move best_move2 = minmax(&state, 4, 0);

  printf("Best move: %d\n", best_move2.move.m);

  print_hex_grid(g);

}

int main() {
  srand(time(NULL));
  test_minmax();
  return 0;
}*/

/*
int main() {
  srand(time(NULL));
  test_astar();
  // test_minmax();
  return 0;
}
*/

// calculer le temps pour chaque algo
void test_AstarVSdIJKSTRA() {
  struct graph_t *g = createGraph(5, TRIANGULAR);
  g->num_objectives = 1;
  // g->objectives     = malloc(sizeof(vertex_t) * 1);
  g->objectives[0]  = g->num_vertices - 10;

  vertex_t start        = 1;
  g->start[0]           = start;
  vertex_t objective    = g->objectives[0];
  g->start[1]           = objective;
  vertex_t opponent_pos = 54;
  vertex_t last_pos     = 0;

  vertex_t* path = malloc(g->num_vertices * sizeof(vertex_t));

  clock_t start_time, end_time;
  printf("previous position  : %d\n", last_pos);
  // print_hex_grid(g);

  start_time          = clock();
  int result_dijkstra = shortest_path_length(g, start, objective, opponent_pos, path, last_pos);
  end_time            = clock();

  double time_dijkstra = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  printf("Dijkstra: Shortest path length: %d\n", result_dijkstra + 1);
  printf("0-->");
  for (int i = 0; i < result_dijkstra + 1; ++i) {
    printf("-->%d ", path[i]);
  }
  printf("\n");

  printf("Dijkstra: Time taken: %f seconds\n", time_dijkstra);

  start_time       = clock();
  int result_astar = shortest_path_astar(g, start, objective, opponent_pos, path, last_pos);
  end_time         = clock();

  double time_astar = (double)(end_time - start_time) / CLOCKS_PER_SEC;

  printf("A*: Shortest path length: %d\n", result_astar + 1);
  printf("0-->");
  for (int i = 0; i < result_astar + 1; ++i) {
    printf("-->%d ", path[i]);
  }
  printf("\n");
  printf("A*: Time taken: %f seconds\n", time_astar);
  free(path);
  graph_free(g);
}
/*
int main() {
  srand(time(NULL));
  test_AstarVSdIJKSTRA();
  return 0;
}
*/