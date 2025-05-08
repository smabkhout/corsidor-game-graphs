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

struct game_state {
  struct graph_t *graph;
  struct move_t   previous_moves[2];  // dernier coup pour chaque joueur
  vertex_t        previous_positions[2];
};

struct scored_move {
  int           score;
  struct move_t move;
};

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
    index_to_axial(v, m, &l, &c);
    if (!in_hexagon(l, c, m, 0, 0)) {
      nodes[v].vertex = -1;
      continue;
    }
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

// Fonction wrapper pour un joueur
int player_shortest_path_length(struct graph_t *g, struct player_tt *p, const vertex_t *objectives,
                                size_t nb_obj, vertex_t opponent_pos, vertex_t *path) {
  int min_length = INT_MAX;

  for (size_t i = 0; i < nb_obj; ++i) {
    int length = shortest_path_length(g, p->position, objectives[i], opponent_pos, path,
                                      0);  // 0 just for the current error
    if (length != -1 && length < min_length) {
      min_length = length;
    }
  }

  return (min_length == INT_MAX) ? -1 : min_length;
}

int evaluate(struct game_state *state, int color) {
  vertex_t *vertex = malloc(100 * sizeof(vertex_t));
  return shortest_path_length(state->graph, state->previous_moves[color].m,
                              state->graph->objectives[0], state->previous_moves[1 - color].m,
                              vertex, 0) -
         shortest_path_length(state->graph, state->previous_moves[1 - color].m,
                              state->graph->objectives[0], state->previous_moves[color].m, vertex,
                              0);
}

int is_game_over(struct game_state *state) {
  for (int i = 0; i < 2; i++) {
    if (state->previous_moves[i].m == state->graph->objectives[0]) {
      return 1;  // Jeu terminé
    }
  }
  return 0;  // Jeu non terminé
}

struct scored_move negamax_ab(struct game_state *state, int depth, int alpha, int beta, int color) {
  if (depth == 0 || is_game_over(state)) {
    int score = evaluate(state, color);
    return (struct scored_move){.score = score};
  }

  int self     = (color == 1) ? 0 : 1;
  int opponent = 1 - self;

  struct move_t    legal_moves[128];
  struct player_tt player = {.position      = state->previous_moves[self].m,
                             .last_position = state->previous_positions[self],
                             .c             = self};
  int              num_moves =
      availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

  struct scored_move best = {.score = -1000000};

  for (int i = 0; i < num_moves; i++) {
    struct game_state  next   = applyy_move(state, legal_moves[i]);
    struct scored_move result = negamax_ab(&next, depth - 1, -beta, -alpha, -color);
    int                score  = -result.score;

    if (score > best.score) {
      best.score = score;
      best.move  = legal_moves[i];
    }

    if (score > alpha)
      alpha = score;

    if (alpha >= beta)
      break;  // Coupure alpha-bêta
  }

  return best;
}

struct scored_move negamax_naive(struct game_state *state, int depth, int color) {
  if (depth == 0 || is_game_over(state)) {
    int score = evaluate(state, color);  // Supposé relatif au joueur color
    return (struct scored_move){.score = score};
  }

  int self     = (color == 1) ? 0 : 1;
  int opponent = 1 - self;

  struct move_t    legal_moves[128];
  struct player_tt player = {.position      = state->previous_moves[self].m,
                             .last_position = state->previous_positions[self],
                             .c             = self};
  int              num_moves =
      availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

  struct scored_move best = {.score = -1000000};

  for (int i = 0; i < num_moves; i++) {
    struct game_state  next   = applyy_move(state, legal_moves[i]);
    struct scored_move result = negamax_naive(&next, depth - 1, -color);
    int                score  = -result.score;  // NegaMax : inversion du signe

    if (score > best.score) {
      best.score = score;
      best.move  = legal_moves[i];
    }
  }

  return best;
}

struct scored_move minMax(struct game_state *state, int depth,
                          enum player_color_t maximizingPlayer) {
  enum player_color_t self     = maximizingPlayer;
  enum player_color_t opponent = 1 - self;
  (void)opponent;
  if (depth == 0 || is_game_over(state)) {
    int score = evaluate(state, self);  // Score du point de vue du MAX
    return (struct scored_move){.move = state->previous_moves[self], .score = score};
  }

  struct move_t    legal_moves[128];
  struct player_tt player = {.position      = state->previous_moves[self].m,
                             .last_position = state->previous_positions[self],
                             .c             = self};
  int num_moves = availableMoves(legal_moves, state->graph, &player, state->previous_moves[self].m);

  struct scored_move best;
  if (!maximizingPlayer) {
    best.score = -1000000;
  } else {
    best.score = 1000000;
  }

  for (int i = 0; i < num_moves; i++) {
    struct game_state next = applyy_move(state, legal_moves[i]);
    // test print
    // printf("move a traitee %d\n" ,legal_moves[i].m) ;
    struct scored_move result = minMax(&next, depth - 1, 1 - maximizingPlayer);
    // printf("score %d \n" , result.score) ;
    if (maximizingPlayer) {
      if (result.score > best.score) {
        best.score = result.score;
        best.move  = legal_moves[i];
      }
    } else {
      if (result.score < best.score) {
        best.score = result.score;
        best.move  = legal_moves[i];
      }
    }
    // print old score and new if it's updated
  }

  return best;
}

#define INF 1000000
struct scored_move minMaxi(struct game_state *state, int depth, int maximizingPlayer,
                           int originalMaxPlayer) {
  // Déterminer les identités des joueurs
  int current_player = maximizingPlayer ? originalMaxPlayer : (1 + originalMaxPlayer) % 2;
  int opponent       = (1 + originalMaxPlayer) % 2;

  // Condition d'arrêt : profondeur nulle ou jeu terminé
  if (depth == 0 || is_game_over(state)) {
    int score = evaluate(state,
                         originalMaxPlayer);  // Évaluation toujours du point de vue du joueur MAX
    return (struct scored_move){.move = state->previous_moves[originalMaxPlayer], .score = score};
  }

  // Génération des coups légaux
  struct move_t    legal_moves[128];
  struct player_tt player = {
      .position      = state->previous_moves[current_player].m,
      .last_position = state->previous_positions[current_player],
      .c             = current_player,
  };

  int num_moves =
      availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

  // Initialisation du meilleur coup
  struct scored_move best_move = {.score = maximizingPlayer ? INT_MIN : INT_MAX};

  // Exploration récursive des coups
  for (int i = 0; i < num_moves; i++) {
    struct game_state next_state = applyy_move(state, legal_moves[i]);
    // players 1 ou 2
    struct scored_move current_result =
        minMaxi(&next_state, depth - 1, !maximizingPlayer, originalMaxPlayer);
    // Mise à jour du meilleur coup
    if ((maximizingPlayer && current_result.score > best_move.score) ||
        (!maximizingPlayer && current_result.score < best_move.score)) {
      best_move.score = current_result.score;
      best_move.move  = legal_moves[i];
    }
  }

  return best_move;
}
