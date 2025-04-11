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
struct move_t create_move(enum player_color_t color, enum move_type_t type, vertex_t vertex, struct edge_t edges[2]);

// Fonction pour vérifier si un mur peut être placé.
// Vérifie si les arêtes spécifiées sont valides dans le graphe donné.
// Retourne 1 si le mur peut être placé, sinon 0.
int can_place_wall(struct graph_t * graph, struct edge_t e[2]);

// Fonction pour vérifier si une position est vide.
// Vérifie si la position 'n' n'est pas occupée par un autre joueur à 'pos_other_player'.
// Retourne 1 si la position est vide, sinon 0.
int is_empty_position(vertex_t n, vertex_t pos_other_player);



 //Vérifie si deux sommets d'un graphe sont connectés.
 //Cette fonction consulte la matrice d'adjacence du graphe pour déterminer s'il existe une arête entre les sommets `v1` et `v2`.
 //return 1 si les sommets sont connectés, 0 sinon.
int is_connected(struct graph_t *graph, vertex_t v1, vertex_t v2);

// Fonction pour vérifier si un joueur peut se déplacer.
// Vérifie si le joueur à 'pos_player' peut se déplacer vers 'b' sans entrer en collision avec un autre joueur.
// Retourne 1 si le mouvement est possible, sinon 0.
int can_move(struct graph_t * graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player);

// Fonction pour vérifier si un mouvement est valide.
// Vérifie si le mouvement donné est valide dans le contexte du graphe.
// Retourne 1 si le mouvement est valide, sinon 0.
int is_valid_move(const struct move_t* move, const struct graph_t* graph);



int distance_minimal(struct graph_t * graph, int d[], int visited[], unsigned int n);
void dijistra ( struct graph_t * graph, vertex_t a, vertex_t b, int d[graph->num_vertices], int prev[graph->num_vertices]);
vertex_t find_closest_objective(struct graph_t* graph, vertex_t player_pos);
enum dir_t get_direction(vertex_t from, vertex_t to, struct graph_t* graph) ;
struct move_t make_move_move(enum player_color_t color, vertex_t dest);
int is_path_clear(struct graph_t* graph, vertex_t from, enum dir_t dir, int dist, vertex_t opponent_pos, vertex_t* result);

void get_side_dir_30(enum dir_t dir, enum dir_t* d1, enum dir_t* d2);
struct move_t find_best_move(struct graph_t* graph, vertex_t pos, vertex_t opponent, enum dir_t prev_dir, enum player_color_t color);
#endif // MOVE_H
