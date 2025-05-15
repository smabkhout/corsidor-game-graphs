#include "strategie3.h"
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <gsl/gsl_spblas.h>
#include <math.h>

#define NUM_DIRECTIONS 6

struct move_t create_move(enum player_color_t color, enum move_type_t type, vertex_t vertex,
                          struct edge_t edges[2]) {
  struct move_t move;
  move.c = color;
  move.t = type;
  move.m = vertex;
  if (type == WALL) {
    move.e[0] = edges[0];
    move.e[1] = edges[1];
  }
  return move;
}

int is_empty_vertice(vertex_t n, vertex_t pos_other_player) {
  if (pos_other_player == n) {
    return 0;
  }
  return 1;
}

int is_valid_move(const struct move_t* move, const struct graph_t* graph) {
  if (!move || !graph) {
    return 0;
  }
  if (move->t == MOVE) {
    if (move->m >= graph->num_vertices) {
      return 0;
    }
  } else if (move->t == WALL) {
    if (move->e[0].fr >= graph->num_vertices || move->e[0].to >= graph->num_vertices ||
        move->e[1].fr >= graph->num_vertices || move->e[1].to >= graph->num_vertices) {
      return 0;
    }
  }
  return 1;
}

int can_move(struct graph_t* graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player) {
  if (!(is_empty_vertice(b, pos_other_player)) ||
      !(gsl_spmatrix_uint_get(graph->t, pos_player, b) > 0)) {
    return 0;
  }
  struct move_t move = create_move(NO_COLOR, MOVE, b, NULL);
  if (!is_valid_move(&move, graph)) {
    return 0;
  }
  return 1;
}

int is_connected(struct graph_t* graph, vertex_t v1, vertex_t v2) {
  if (!graph) {
    return 0;
  }
  if (gsl_spmatrix_uint_get(graph->t, v1, v2) > 0) {
    return 1;
  }
  return 0;
}

int can_place_wall(struct graph_t* graph, struct edge_t e[2]) {
  if (e[0].fr != e[1].fr) {
    return 0;
  }
  if (!is_connected(graph, e[0].fr, e[0].to) || !is_connected(graph, e[1].fr, e[1].to)) {
    return 0;
  }
  struct move_t move = create_move(NO_COLOR, WALL, 0, e);
  if (!is_valid_move(&move, graph)) {
    return 0;
  }
  return 1;
}

int distance_minimal(int d[], int visited[], unsigned int n) {
  int          min   = INT_MAX;
  unsigned int index = -1;
  for (unsigned int i = 0; i < n; i++) {
    if (!visited[i] && d[i] <= min) {
      min   = d[i];
      index = i;
    }
  }
  return index;
}

void dijistra(struct graph_t* graph, vertex_t a, vertex_t b, int d[graph->num_vertices],
              int prev[graph->num_vertices], int next[graph->num_vertices],
              vertex_t pos_other_player) {
  int visited[graph->num_vertices];
  for (unsigned int i = 0; i < graph->num_vertices; i++) {
    d[i]       = INT_MAX;
    prev[i]    = -1;
    visited[i] = 0;
    next[i]    = -1;
  }
  d[a] = 0;
  while (1) {
    unsigned int index_min = distance_minimal(d, visited, graph->num_vertices);
    if (index_min == (unsigned int)-1) {
      break;
    }
    visited[index_min] = 1;
    if (index_min == b) {
      break;
    }
    for (unsigned int j = 0; j < graph->num_vertices; j++) {
      int dir   = gsl_spmatrix_uint_get(graph->t, index_min, j);
      int poids = 1;
      if (dir > 0 && dir != 7 && d[index_min] + poids < d[j] && !visited[j] &&
          can_move(graph, index_min, j, pos_other_player)) {
        d[j]    = d[index_min] + poids;
        prev[j] = index_min;
      }
    }
  }
  if (prev[b] != -1 || a == b) {
    int path[graph->num_vertices];
    int len = 0;
    for (vertex_t v = b; v != (vertex_t)-1; v = prev[v]) {
      path[len++] = v;
    }
    if (len > 1) {
      next[a] = path[len - 2];
    } else {
      next[a] = a;
    }
  } else {
    // b inaccessible depuis a
    next[a] = -1;
  }
}

void calculate_dist_objectives(struct graph_t* graph, int num_objectives,
                               int      distance[num_objectives][num_objectives],
                               vertex_t pos_other_player) {
  for (int i = 0; i < num_objectives; i++) {
    for (int j = 0; j < num_objectives; j++) {
      if (i == j) {
        distance[i][j] = 0;
      } else {
        int d[graph->num_vertices];
        int prev[graph->num_vertices];
        int next[graph->num_vertices];
        dijistra(graph, graph->objectives[i], graph->objectives[j], d, prev, next,
                 pos_other_player);
        distance[i][j] = d[graph->objectives[j]];
      }
    }
  }
}

// fonction qui calcule la distance totale pour une permutation
int calculate_total_dist(int n, int d[n][n], int tab[]) {
  int t = 0;
  for (int i = 0; i < n - 1; i++) {
    t += d[tab[i]][tab[i + 1]];
  }
  t += d[tab[n - 1]][tab[0]];
  return t;
}

// fonction pour generer next permutation
int next_permutation(int arr[], int n) {
  int i = n - 1;
  // Trouver l'élément qui n'est pas dans l'ordre décroissant
  while (i > 0 && arr[i - 1] >= arr[i]) i--;
  // Si tous les éléments sont dans l'ordre décroissant, c'est la dernière permutation
  if (i <= 0)
    return 0;
  // Trouver l'élément le plus grand à droite de i-1 mais plus grand que arr[i-1]
  int j = n - 1;
  while (arr[j] <= arr[i - 1]) j--;
  // Échanger arr[i-1] et arr[j]
  int a      = arr[i - 1];
  arr[i - 1] = arr[j];
  arr[j]     = a;
  // Inverser la séquence après la position i-1
  j = n - 1;
  while (i < j) {
    a      = arr[i];
    arr[i] = arr[j];
    arr[j] = a;
    i++;
    j--;
  }
  return 1;
}

int exist_in_array(int a, int n, int t[]) {
  for (int i = 0; i < n; i++) {
    if (t[i] == a) {
      return 1;
    }
  }
  return 0;
}

int len(int n, int t[]) {
  int s = 0;
  for (int i = 0; i < n; i++) {
    if (t[i] != -1) {
      s++;
    }
  }
  return s;
}

// probléme du voyageur de Commerce
int TSP(struct graph_t* graph, int best_order[], int obj_visited[], vertex_t pos_other_player) {
  int d[graph->num_objectives][graph->num_objectives];
  calculate_dist_objectives(graph, graph->num_objectives, d, pos_other_player);

  int n = graph->num_objectives;
  int remaining[n];
  int num_remaining = 0;
  for (int i = 0; i < n; i++) {
    if (!exist_in_array(i, n, obj_visited)) {
      remaining[num_remaining++] = i;
    }
  }

  if (num_remaining == 0) {
    return 0;  // tous les objectifs sont déjà atteints
  }

  int perm[num_remaining];
  for (int i = 0; i < num_remaining; i++) {
    perm[i] = i;
  }

  int best_temp[num_remaining];
  int min_distance = INT_MAX;
  do {
    int cost = calculate_total_dist(num_remaining, d, perm);
    if (cost < min_distance) {
      min_distance = cost;
      memcpy(best_temp, perm, sizeof perm);
    }
  } while (next_permutation(perm, num_remaining));

  for (int i = 0; i < num_remaining; i++) {
    best_order[i] = remaining[best_temp[i]];
  }

  return min_distance;
}

vertex_t find_closest_objective(struct graph_t* graph, vertex_t player_pos,
                                vertex_t pos_other_player) {
  vertex_t*    objectives        = graph->objectives;
  unsigned int num_obj           = graph->num_objectives;
  int          min_distance      = INT_MAX;
  vertex_t     closest_objective = player_pos;
  int          distances[graph->num_vertices];
  int          prev[graph->num_vertices];
  int          next[graph->num_vertices];
  for (unsigned int i = 0; i < num_obj; i++) {
    vertex_t objective = objectives[i];
    dijistra(graph, player_pos, objective, distances, prev, next, pos_other_player);
    int obj_distance = distances[objective];

    if (obj_distance < min_distance) {
      min_distance      = obj_distance;
      closest_objective = objective;
    }
  }
  return closest_objective;
}

// renvoie le nombre de voisins
int num_neighbors(struct graph_t* graph, vertex_t v, vertex_t* out, int max_out) {
  if (v >= graph->num_vertices)
    return 0;
  int count = 0;
  for (vertex_t i = 0; i < graph->num_vertices && count < max_out; i++) {
    if (gsl_spmatrix_uint_get(graph->t, v, i) != 0) {
      out[count++] = i;
    }
  }
  return count;
}

struct move_t try_place_wall(struct graph_t* graph, vertex_t pos_enemy, vertex_t next_enemy,
                             struct move_t fallback) {
  struct edge_t wall[2];
  wall[0].fr = pos_enemy;
  wall[0].to = next_enemy;
  vertex_t nbr;
  vertex_t tmp[6];  // au max 6 voisins
  int      nb = num_neighbors(graph, pos_enemy, tmp, 6);
  nbr         = (vertex_t)-1;
  for (int i = 0; i < nb; ++i) {
    if (tmp[i] != next_enemy) {
      nbr = tmp[i];
      break;
    }
  }
  if (nbr == (vertex_t)-1) {
    return fallback;
  }
  wall[1].fr = pos_enemy;
  wall[1].to = nbr;

  if (!can_place_wall(graph, wall)) {
    return fallback;
  }
  return (struct move_t){.t = WALL, .c = fallback.c, .e = {wall[0], wall[1]}};
}
