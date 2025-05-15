#ifndef _CORS_BOARD_H_
#define _CORS_BOARD_H_

#include "graph_functions.h"

/*
 * Structure représentant l’état du plateau de jeu.
 * - moves : liste des coups joués
 * - wall_count : nombre de murs posés
 * - graph : pointeur vers le graphe de jeu
 * - size_moves : taille actuelle du tableau moves
 * - current_positions : positions actuelles des deux pions
 */
struct board_t {
  struct move_t*  moves;
  int             wall_count;
  struct graph_t* graph;
  int             size_moves;
  vertex_t        current_positions[2];
};

/*
 * Initialise un nouveau plateau de jeu.
 * - Alloue et retourne un pointeur vers board_t avec champs initialisés.
 */
struct board_t* board_init();

/*
 * Ajoute un coup au plateau.
 * - Met à jour moves, size_moves et wall_count si nécessaire.
 * - Met à jour current_positions du joueur concerné.
 */
void add_move_to_board(struct board_t* board, struct move_t move);

/*
 * Libère la mémoire associée au plateau.
 * - Désalloue moves et le graphe embarqué, puis le struct board_t.
 */
void board_free(struct board_t* board);

/*
 * Vérifie si un coup est invalide.
 * - Teste la validité d’un MOVE ou d’un WALL dans le contexte du plateau.
 * - Retourne 1 si le coup est invalide, 0 sinon.
 */
int is_invalid(struct move_t move, struct board_t* board);

/*
 * Vérifie s’il est possible de parcourir dist arêtes dans la direction dir
 * à partir de from sans rencontrer de mur ni l’adversaire.
 * - Si réussi, stocke le sommet d’arrivée dans *result et retourne 1.
 * - Sinon retourne 0.
 */
int is_path_clear(struct graph_t* graph, vertex_t from, enum dir_t dir, int dist,
                  vertex_t opponent_pos, vertex_t* result);

/*
 * Calcule les deux directions adjacentes à ±30° autour de dir.
 * - Renvoie dans *d1 et *d2 les directions voisines.
 */
void get_side_dir_30(enum dir_t dir, enum dir_t* d1, enum dir_t* d2);

/*
 * Propose un coup de repli si Dijkstra échoue.
 * - Essaie d’avancer dans prev_dir en sautant l’adversaire ou en ligne droite.
 * - Retourne un MOVE vers une case valide, ou NO_TYPE si aucun mouvement possible.
 */
struct move_t find_best_move(struct graph_t* graph, vertex_t pos, vertex_t opponent,
                             enum dir_t prev_dir, enum player_color_t color);

/*
 * Crée un coup MOVE simple vers dest.
 */
struct move_t make_move_move(enum player_color_t color, vertex_t dest);

/*
 * Récupère les voisins directs d’un sommet.
 * - Remplit out[] avec jusqu’à max_out voisins.
 * - Retourne le nombre de voisins trouvés.
 */
int get_neighbors(struct graph_t* graph, vertex_t v, vertex_t* out, int max_out);

#endif  // _CORS_BOARD_H_
