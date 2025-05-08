#include "graph.h"
#include "player.h"
#include "board.h"
#include "move2.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

static struct board_t *board = NULL;
static unsigned int    player_id;
static vertex_t        initial_position;
static int             visited_objectives[256] = {0};
static vertex_t        my_last_position        = (vertex_t)-1;
// static vertex_t last_positions[3] = { -1, -1, -1 };

int s_p(struct graph_t *g, vertex_t start, vertex_t goal) {
  int *visited = calloc(g->num_vertices, sizeof(int));
  int *dist    = malloc(g->num_vertices * sizeof(int));
  if (!visited || !dist) {
    fprintf(stderr, "Erreur d'allocation mémoire dans s_p\n");
    exit(EXIT_FAILURE);
  }

  for (vertex_t i = 0; i < g->num_vertices; i++) {
    dist[i] = INT_MAX;
  }
  dist[start] = 0;

  for (vertex_t i = 0; i < g->num_vertices; i++) {
    int u = -1;
    for (vertex_t j = 0; j < g->num_vertices; j++) {
      if (!visited[j] && (u == -1 || dist[j] < dist[(vertex_t)u])) {
        u = j;
      }
    }

    if (u == -1)
      break;
    if (dist[(vertex_t)u] == INT_MAX || (vertex_t)u == goal)
      break;

    visited[(vertex_t)u] = 1;

    for (vertex_t v = 0; v < g->num_vertices; v++) {
      if (gsl_spmatrix_uint_get(g->t, (vertex_t)u, v) > 0 && !visited[v]) {
        int alt = dist[(vertex_t)u] + 1;
        if (alt < dist[v]) {
          dist[v] = alt;
        }
      }
    }
  }

  int result = dist[goal];
  free(visited);
  free(dist);
  return result == INT_MAX ? -1 : result;
}

int all_objectives_visited(struct graph_t *g) {
  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    if (!visited_objectives[g->objectives[i]])
      return 0;
  }
  return 1;
}

vertex_t get_next_closest_objective(struct graph_t *g, vertex_t current) {
  int      min_dist = INT_MAX;
  vertex_t closest  = current;
  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    vertex_t obj = g->objectives[i];
    if (!visited_objectives[obj]) {
      int dist = s_p(g, current, obj);
      if (dist >= 0 && dist < min_dist) {
        min_dist = dist;
        closest  = obj;
      }
    }
  }
  return closest;
}

struct move_t make_move_no_type() {
  struct move_t move;
  move.t       = NO_TYPE;
  move.c       = NO_COLOR;
  move.m       = 0;
  move.e[0].fr = move.e[0].to = 0;
  move.e[1].fr = move.e[1].to = 0;
  return move;
}

char const *get_player_name() {
  return "Jennie";
}

void initialize(unsigned int id, struct graph_t *graph) {
  board        = board_init();
  board->graph = graph;
  if (!board->graph) {
    fprintf(stderr, "Erreur allocation du graph\n");
    exit(EXIT_FAILURE);
  }
  player_id                   = id;
  initial_position            = graph->start[id];
  board->current_positions[0] = graph->start[0];
  board->current_positions[1] = graph->start[1];
  my_last_position            = initial_position;
  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n",
         id, board->graph->num_vertices, board->graph->num_edges, board->graph->num_objectives);
}

const struct axial_t blackpink[7] = {
    {0, 0},   // No edge
    {1, -1},  // NW
    {1, 0},   // NE
    {0, 1},   // E
    {-1, 1},  // SE
    {-1, 0},  // SW
    {0, -1}   // W
};

/*int get_direction(vertex_t from, vertex_t to, struct graph_t* g) {
    if (!g || !g->t) return -1;

    // Cas simple : la direction est déjà présente dans la matrice
    unsigned int val = gsl_spmatrix_uint_get(g->t, from, to);
    if (val != 0 && val != 7)  // 7 = mur
        return (int)val;

    // Cas plus général : déduction via les coordonnées axiales
    int m = 0;
    in_hexagon_func_t in_hex = NULL;
    resolve_graph_type_or_default(g, &m, &in_hex); // récupère m automatiquement

    int lf, cf, lt, ct;
    index_to_axial(from, m, &lf, &cf);
    index_to_axial(to,   m, &lt, &ct);

    int dl = lt - lf;
    int dc = ct - cf;

    for (int dir = 1; dir <= 6; ++dir) {
        if (blackpink[dir].l == dl && blackpink[dir].c == dc)
            return dir;
    }

    return -1;
}*/

void index_to_axial_developped(const struct graph_t *g, vertex_t index, int *l, int *c) {
  int               m      = 0;
  in_hexagon_func_t in_hex = NULL;
  resolve_graph_type_or_default((struct graph_t *)g, &m, &in_hex);

  for (int i = 1 - m; i < m; ++i) {
    for (int j = 1 - m; j < m; ++j) {
      if (!in_hex(i, j, m, 0, 0))
        continue;

      int idx = axial_to_index(i, j, m);
      if ((vertex_t)idx == index) {
        *l = i;
        *c = j;
        return;
      }
    }
  }

  fprintf(stderr, "index %u not found in graph with m = %d\n", index, m);
  *l = *c = 0;
}

int get_direction(vertex_t from, vertex_t to, struct graph_t *g) {
  if (!g || !g->t)
    return -1;
  unsigned int val = gsl_spmatrix_uint_get(g->t, from, to);
  if (val != 0 && val != 7)
    return (int)val;
  int               m      = 0;
  in_hexagon_func_t in_hex = NULL;
  resolve_graph_type_or_default((struct graph_t *)g, &m, &in_hex);

  int lf, cf, lt, ct;
  index_to_axial_developped(g, from, &lf, &cf);
  index_to_axial_developped(g, to, &lt, &ct);

  int dl = lt - lf;
  int dc = ct - cf;

  for (int dir = 1; dir <= 6; ++dir) {
    int l_step = blackpink[dir].l;
    int c_step = blackpink[dir].c;

    int factor = 0;
    if (l_step == 0 && dl == 0 && c_step != 0)
      factor = dc / c_step;
    else if (c_step == 0 && dc == 0 && l_step != 0)
      factor = dl / l_step;
    else if (l_step != 0 && c_step != 0 && dl % l_step == 0 && dc % c_step == 0 &&
             dl / l_step == dc / c_step)
      factor = dl / l_step;

    if (factor > 0 && factor <= 3)
      return dir;
  }

  return -1;
}

struct move_t play(const struct move_t previous_move) {
  if (previous_move.t == MOVE) {
    board->current_positions[previous_move.c] = previous_move.m;
  }

  struct graph_t *g        = board->graph;
  vertex_t        my_pos   = board->current_positions[player_id];
  vertex_t        last_pos = my_last_position;
  vertex_t        opp_pos  = board->current_positions[(player_id + 1) % 2];

  printf("Jennie DEBUG -- position = %u, last_position = %u\n", my_pos, last_pos);

  if (previous_move.t == WALL && previous_move.c != player_id) {
    unsigned int *temp  = gsl_spmatrix_uint_ptr(g->t, previous_move.e[0].fr, previous_move.e[0].to);
    *temp               = 7;
    unsigned int *temp1 = gsl_spmatrix_uint_ptr(g->t, previous_move.e[1].fr, previous_move.e[1].to);
    *temp1              = 7;
    unsigned int *temp2 = gsl_spmatrix_uint_ptr(g->t, previous_move.e[0].to, previous_move.e[0].fr);
    *temp2              = 7;
    unsigned int *temp3 = gsl_spmatrix_uint_ptr(g->t, previous_move.e[1].to, previous_move.e[1].fr);
    *temp3              = 7;
    struct player_tt dummy = {
        .position = opp_pos, .last_position = opp_pos, .walls = 10, .c = (player_id + 1) % 2};
    place_wall(g, &dummy, previous_move);
    printf("Mur adverse détecté : entre %u-%u et %u-%u\n", previous_move.e[0].fr,
           previous_move.e[0].to, previous_move.e[1].fr, previous_move.e[1].to);
  }

  if (my_pos == initial_position && all_objectives_visited(g)) {
    printf("Bingooooo. Jennie did it and won\n");
    struct move_t win_move = make_move_no_type();
    win_move.t             = MOVE;
    win_move.m             = my_pos;
    win_move.c             = player_id;
    return win_move;
    // return make_move_no_type();
  }

  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    if (my_pos == g->objectives[i]) {
      visited_objectives[my_pos] = 1;
      break;
    }
  }

  vertex_t target =
      all_objectives_visited(g) ? initial_position : get_next_closest_objective(g, my_pos);

  struct player_tt player = {
      .position = my_pos, .last_position = last_pos, .walls = 10, .c = player_id};
  enum dir_t last_dir = get_direction(last_pos, my_pos, g);

  struct move_t best_move = {.t = NO_TYPE};
  int           best_dist = INT_MAX;
  struct move_t options[100];
  int           nb = availableMovess(options, g, &player, opp_pos);

  for (int i = 0; i < nb; ++i) {
    struct move_t move     = options[i];
    enum dir_t    move_dir = get_direction(my_pos, move.m, g);
    if ((int)move_dir == -1)
      continue;

    int allowed_steps = 1;
    if ((int)last_dir != -1) {
      if (move_dir == last_dir) {
        allowed_steps = 3;
      } else if ((move_dir + 1) % 6 == last_dir || (move_dir + 5) % 6 == last_dir) {
        allowed_steps = 2;
      }
    }

    int distance = s_p(g, my_pos, move.m);
    if (distance > allowed_steps)
      continue;

    struct player_tt p_sim = player;
    if (!apply_move(g, &p_sim, move, opp_pos))
      continue;

    int dist = s_p(g, p_sim.position, target);
    if (dist >= 0 && dist < best_dist) {
      best_move = move;
      best_dist = dist;
    }
  }

  if (best_move.t == NO_TYPE && player.walls > 0) {
    // Code pour tenter de bloquer l'adversaire (comme avant)
    vertex_t adv_pos     = opp_pos;
    vertex_t adv_target  = get_next_closest_objective(g, adv_pos);
    int      dist_before = s_p(g, adv_pos, adv_target);

    if (dist_before == 1) {
      int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
      int l, c;
      index_to_axial(adv_pos, m, &l, &c);

      for (int d = 1; d < 7; ++d) {
        int l1 = l + blackpink[d].l;
        int c1 = c + blackpink[d].c;
        if (!in_hexagon_T(l1, c1, m, 0, 0))
          continue;

        vertex_t to  = axial_to_index(l1, c1, m);
        vertex_t fr1 = adv_pos, to1 = to;
        vertex_t fr2 = adv_pos, to2 = to;

        for (int d2 = 1; d2 < 7; ++d2) {
          if (d2 == d)
            continue;
          int l2 = l + blackpink[d2].l;
          int c2 = c + blackpink[d2].c;
          if (!in_hexagon_T(l2, c2, m, 0, 0))
            continue;
          to2 = axial_to_index(l2, c2, m);
          break;
        }

        struct move_t *wall_ptr = make_wall_move(player_id, fr1, to1, fr2, to2);
        struct move_t  wall     = *wall_ptr;
        free(wall_ptr);

        if (valid_wall(g, &player, wall)) {
          struct graph_t g_copy;
          copy_graph(&g_copy, g);
          place_wall(&g_copy, &player, wall);
          int dist_after = s_p(&g_copy, adv_pos, adv_target);

          if (dist_after > dist_before &&
              path_to_objective_exists(&g_copy, adv_pos, g->objectives, g->num_objectives)) {
            best_move = wall;
            printf("Jennie puts a wall beetween %u-%u and %u-%u\n", wall.e[0].fr, wall.e[0].to,
                   wall.e[1].fr, wall.e[1].to);
            free(g_copy.objectives);
            free(g_copy.t->data);
            gsl_spmatrix_uint_free(g_copy.t);
            break;
          }

          free(g_copy.objectives);
          free(g_copy.t->data);
          gsl_spmatrix_uint_free(g_copy.t);
        }
      }
    }
  }

  add_move_to_board(board, best_move);

  if (best_move.t == MOVE) {
    my_last_position                    = my_pos;
    board->current_positions[player_id] = best_move.m;
    printf(" Jennie se déplace de %u à %u\n", my_last_position, best_move.m);
  } else if (best_move.t == WALL) {
    printf(" Mur validé pour Jennie\n");
  } else {
    printf(" Aucun coup trouvé — Jennie passe\n");
  }

  return best_move;
}

void finalize() {
  if (board) {
    board_free(board);
    board = NULL;
  }
}
