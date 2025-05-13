#ifndef MOVE_H
#define MOVE_H

#include "graph.h"
#include "move.h"
#include <stdlib.h>
#include <stdio.h>

// Fonction pour créer un mouvement.
// Prend en entrée la couleur du joueur, le type de mouvement (déplacement ou mur),
// la position du mouvement, et les arêtes associées si c'est un mur.
// Retourne un objet de type 'move_t' représentant le mouvement créé.
struct move_t create_move(enum player_color_t color, enum move_type_t type, vertex_t vertex,
                          struct edge_t edges[2]);

// Fonction pour vérifier si un mur peut être placé.
// Vérifie si les arêtes spécifiées sont valides dans le graphe donné.
// Retourne 1 si le mur peut être placé, sinon 0.
int can_place_wall(struct graph_t* graph, struct edge_t e[2]);

// Fonction pour vérifier si une position est vide.
// Vérifie si la position 'n' n'est pas occupée par un autre joueur à 'pos_other_player'.
// Retourne 1 si la position est vide, sinon 0.
int is_empty_vertice(vertex_t n, vertex_t pos_other_player);

// Vérifie si deux sommets d'un graphe sont connectés.
// Cette fonction consulte la matrice d'adjacence du graphe pour déterminer s'il existe une arête
// entre les sommets `v1` et `v2`. return 1 si les sommets sont connectés, 0 sinon.
int is_connected(struct graph_t* graph, vertex_t v1, vertex_t v2);

// Fonction pour vérifier si un joueur peut se déplacer.
// Vérifie si le joueur à 'pos_player' peut se déplacer vers 'b' sans entrer en collision avec un
// autre joueur. Retourne 1 si le mouvement est possible, sinon 0.
int can_move(struct graph_t* graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player);

// Fonction pour vérifier si un mouvement est valide.
// Vérifie si le mouvement donné est valide dans le contexte du graphe.
// Retourne 1 si le mouvement est valide, sinon 0.
int is_valid_move(const struct move_t* move, const struct graph_t* graph);

int  distance_minimal(int d[], int visited[], unsigned int n);
void dijistra(struct graph_t* graph, vertex_t a, vertex_t b, int d[graph->num_vertices],
              int prev[graph->num_vertices], int next[graph->num_vertices],
              vertex_t pos_other_player);
void calculate_dist_objectives(struct graph_t* graph, int num_objectives,
                               int      distance[num_objectives][num_objectives],
                               vertex_t pos_other_player);
int  calculate_total_dist(int n, int d[n][n], int tab[]);
int  next_permutation(int arr[], int n);

int exist_in_array(int a, int n, int t[]);
int TSP(struct graph_t* graph, int best_order[], int obj_visited[], vertex_t pos_other_player);

vertex_t      find_closest_objective(struct graph_t* graph, vertex_t player_pos,
                                     vertex_t pos_other_player);
struct move_t try_place_wall(struct graph_t* graph, vertex_t pos_enemy, vertex_t next_enemy,
                             enum player_color_t my_color);

#endif  // MOVE_H
