#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include <assert.h>
#include <math.h>
#include "graph.h"

// Structure pour représenter des coordonnées axiales dans un pavage hexagonal.
struct axial_t {
  int l;  // ligne
  int c;  // colonne
};

// Conversion coordonnées (l, c) vers un index du graphe pour un pavage triangulaire.
int axial_to_index_T(int l, int c, int m);

// Conversion coordonnées (l, c) vers un index du graphe pour un pavage cyclique.
int axial_to_index_C(int l, int c, int m);

// Conversion coordonnées (l, c) vers un index du graphe pour un pavage troué (holey).
int axial_to_index_H(int l, int c, int m);

// Conversion coordonnées (l, c) vers un index du graphe pour un pavage "span".
int axial_to_index_S(int l, int c, int m);

// Conversion générique selon le type de pavage (TRIANGULAR, CYCLIC, HOLEY, SPAN).
int axial_to_index(int l, int c, int m, int type);

// Vérifie si une coordonnée est dans l’hexagone triangulaire.
int in_hexagon_T(int l, int c, int m, int l_origin, int c_origin);

// Vérifie si une coordonnée est dans l’hexagone cyclique.
int in_hexagon_C(int l, int c, int m, int l_origin, int c_origin);

// Vérifie si une coordonnée est dans l’hexagone troué (holey).
int in_hexagon_H(int l, int c, int m, int l_origin, int c_origin);

// Vérifie si une coordonnée est dans l’hexagone span.
int in_hexagon_S(int l, int c, int m, int l_origin, int c_origin);

// Génère un graphe à partir de m et d’une fonction de filtrage sur les coordonnées.
void graph_generate(int m, struct graph_t *g,
                    int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin));

// Crée un graphe de taille m selon un type de pavage donné.
struct graph_t *createGraph(int m, enum graph_type_t type);

// Affiche la matrice d’adjacence du graphe au format lisible.
void graph_print_matrix(const struct graph_t *g);

// Affiche le graphe pour le débogage (objectifs, positions, etc.).
void graph_print(struct graph_t *graph);

// Copie le contenu d’un graphe source vers un graphe destination.
void copy_graph(struct graph_t *dest, const struct graph_t *src);

// Vérifie si une case contient un objectif ou un joueur.
int is_objective_or_player(int l, int c, int m, struct graph_t *g);

// Libère la mémoire allouée au graphe.
void graph_free(struct graph_t *g);

// Affiche la grille hexagonale du graphe dans le terminal.
void print_hex_grid(struct graph_t *g);

// Conversion index → coordonnées axiales.
void index_to_axial(int index, int m, int *l, int *c, int type);

// Exporte le graphe au format DOT pour visualisation via Graphviz.
void graph_to_dot(const struct graph_t *g, const char *filename);

// Type de fonction utilisée pour tester l’appartenance à un hexagone.
typedef int (*in_hexagon_func_t)(int l, int c, int m, int l_origin, int c_origin);

// Détermine dynamiquement le type de graphe et associe la bonne fonction in_hexagon.
void resolve_graph_type_or_default(struct graph_t *g, int *m, in_hexagon_func_t *in_hexagon);
