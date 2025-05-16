#pragma once

#include "move.h"
#include "graph_functions.h"

// Structure représentant un joueur avec couleur, nombre de murs restants,
// position actuelle et position précédente pour estimer la direction du mouvement.
struct player_tt {
  enum player_color_t c;
  unsigned int        walls;
  vertex_t            position;
  vertex_t            last_position;
};

// Calcule la direction (1 à 6) à partir des différences de coordonnées axiales.
// Sert à déterminer la direction d’un déplacement dans un pavage hexagonal.
int direction_axial(int dl, int dc);

// Vérifie si un mouvement vers une case cible est valide selon les règles du jeu.
// Retourne une distance si le mouvement est autorisé, sinon 0.
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opp);

// Vérifie si un mur est valide (structurellement et par rapport aux règles du jeu).
// Retourne 1 si valide, sinon 0.
int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move);

// Applique un mur sur le graphe et met à jour le nombre de murs du joueur.
void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move);

// Crée un mouvement de type WALL à partir de deux arêtes (chaque mur est composé de 2 arêtes).
struct move_t *make_wall_move(enum player_color_t color, vertex_t fr1, vertex_t to1, vertex_t fr2,
                              vertex_t to2);

// Remplit un tableau de mouvements disponibles depuis la position actuelle du joueur.
// Retourne le nombre de mouvements trouvés.
int availableMoves(struct move_t moves[], struct graph_t *graph, struct player_tt *p,
                   vertex_t opponent);

// Variante alternative de availableMoves (peut être spécifique à un autre mode de déplacement).
int availableMovess(struct move_t moves[], struct graph_t *graph, struct player_tt *p,
                    vertex_t opponent);

// Applique un mouvement (MOVE ou WALL) au joueur et au graphe.
// Retourne 1 si le mouvement est appliqué, 0 sinon.
int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move, vertex_t opp);

// Vérifie s’il existe un chemin valide entre une position et au moins un objectif.
// Retourne 1 si oui, sinon 0.
int path_to_objective_exists(struct graph_t *g, vertex_t start, const vertex_t *objectives,
                             size_t nb_obj);

// Crée un mouvement de type MOVE vers la case `dest`.
struct move_t make_move_moove(enum player_color_t color, vertex_t dest);
