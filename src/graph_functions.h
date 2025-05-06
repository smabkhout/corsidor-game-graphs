#pragma once
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include <assert.h>
#include <math.h>
#include "graph.h"

// Coordonnées axiales pour pavage hexagonal : (0, 0) en centre, (0, 1) vecteur
// déplacement East, (1, 0) vecteur déplacement North East, (1, -1) vecteur
// déplacement North West,
struct axial_t {
  int l;  // ligne
  int c;  // colonne
};

// Conversion coordonnées (l, c) -> index dans le graphe
int axial_to_index(int l, int c, int m);

// Vérifie si (l, c) est bien dans l'hexagone de type triangulaire
int in_hexagon_T(int l, int c, int m, int l_origin, int c_origin);

// Vérifie si (l, c) est bien dans l'hexagone de type cyclique
int in_hexagon_C(int l, int c, int m, int l_origin, int c_origin);

// Vérifie si (l, c) est bien dans l'hexagone de type trouée (HOLEY)
int in_hexagon_H(int l, int c, int m, int l_origin, int c_origin);

void graph_generate(int m, struct graph_t *g,
                    int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin));

// Cree un graphe de type "enum graph_type_t type" et de la variable m "int m"
struct graph_t *createGraph(int m, enum graph_type_t type);

// Affichage formaté de la matrice d'adjacence
void graph_print_matrix(const struct graph_t *g);

// Affichage pour debug
void graph_print(struct graph_t *graph);

void copy_graph(struct graph_t *dest, const struct graph_t *src);

int is_objective_or_player(int l, int c, int m, struct graph_t *g);

// Libération du graphe
void graph_free(struct graph_t *g);

void print_hex_grid(struct graph_t *g);

void index_to_axial(int index, int m, int *l, int *c);

void graph_to_dot(const struct graph_t *g, const char *filename);