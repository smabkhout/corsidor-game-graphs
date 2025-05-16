#include "graph.h"
#include "move2.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

typedef unsigned int vertex_t;

// Structure pour stocker les informations nécessaires à Dijkstra ou A*
struct distance_node {
  vertex_t vertex;     // Sommet concerné
  int      distance;   // Distance calculée depuis le départ
  bool     visited;    // Sommet déjà visité
  int      num_moves;  // Nombre de coups pour l'atteindre
};

// Représente l'état complet du jeu à un instant donné
struct game_state {
  struct graph_t *graph;                  // Le graphe de jeu
  struct move_t   previous_moves[2];      // Dernier coup de chaque joueur
  vertex_t        previous_positions[2];  // Position précédente des joueurs
};

// Structure associant un score à un coup proposé
struct scored_move {
  int           score;  // Score du coup
  struct move_t move;   // Coup proposé
};

// Sélectionne le sommet avec la plus petite distance non encore visité
vertex_t min_distance_vertex(struct distance_node *nodes, size_t num_vertices);

// Calcule la longueur du plus court chemin entre `start` et `objective` avec Dijkstra
int shortest_path_length(struct graph_t *g, vertex_t start, vertex_t objective,
                         vertex_t opponent_pos, vertex_t *path, vertex_t last_pos);

// Calcule le plus court chemin avec A* (plus rapide que Dijkstra selon le cas)
int shortest_path_astar(struct graph_t *g, vertex_t start, vertex_t objective,
                        vertex_t opponent_pos, vertex_t *path, vertex_t last_pos);

// Fonction heuristique utilisée pour A* (estimation de la distance entre deux sommets)
double heuristic(vertex_t a, vertex_t b, int m, int type);

// Applique un coup et retourne un nouvel état de jeu mis à jour
struct game_state applyy_move(const struct game_state *state, struct move_t move);

// Fonction de test comparant les performances d'A* et Dijkstra
void test_AstarVSdIJKSTRA();

// Fonction pour évaluer un état de jeu selon la couleur du joueur
// int evaluate(struct game_state *state, int color);

// Fonction pour explorer l'arbre des coups à profondeur limitée
// struct scored_move minmax(struct game_state *state, int depth, int color);
