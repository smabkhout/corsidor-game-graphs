#include "graph.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*
// Création d'un graphe vide
struct graph_t* graph_create(int num_vertices) {
    struct graph_t* g = malloc(sizeof(struct graph_t));
    if (!g) return NULL;
    g->num_vertices = num_vertices;
    g->num_edges = 0;
    g->t = gsl_spmatrix_uint_alloc(num_vertices, num_vertices);
    if (!g->t) {
        free(g);
        return NULL;
    }
    return g;
}
    */

#include <math.h>

// Coordonnées axiales pour pavage hexagonal
struct axial_t {
  int l; // ligne
  int c; // colonne
};

// Conversion coordonnées (q, r) -> index dans la matrice
int axial_to_index(int l, int c, int m) {
  int i = m - l - 1; // indice de la ligne dans la matrice equivalente à la
                     // notation graphe (son image)
  int j = m + c - 1; // indice de la colonne dans la matrice equivalente à la
                     // notation graphe (son image)
  int count = 0;     // nombre d'elements dans la matrice non representes sur le
                     // graphe (à supprimer de l'indice)
  if (l > 0)
    count = m * (m - 1) / 2 - l * (l + 1) / 2;
  else if (l < 0)
    count = m * (m - 1) / 2 + abs(l) * (abs(l) + 1) / 2;
  else
    count = m * (m - 1) / 2;
  return j + (2 * m - 1) * i - count;
}

// Vérifie si (l, c) est bien dans l'hexagone de type triangulaire
int in_hexagon_T(int l, int c, int m) {
  return (abs(l) <= m - 1) && (abs(c) <= m - 1) && (abs(l + c) <= m - 1);
}

// Vérifie si (l, c) est bien dans l'hexagone de type cyclique
int in_hexagon_C(int l, int c, int m) {
    return (abs(l) == m - 1) || (abs(l) == m - 2)
     || (abs(c) == m - 1) || (abs(c) == m - 1)
      || (abs(l+c) == m - 1) || (abs(l+c) == m - 1);
}

// Vérifie si (l, c) est bien dans l'hexagone de type trouée (HOLEY)
int in_hexagon_H(int l, int c, int m) {
    // à faire ...
    return (abs(l) == m - 1) ;
}

// Vecteurs de directions (en coord. axiales)
const struct axial_t directions[7] = {
    {0, 0},  // No edge
    {1, -1}, // NW
    {1, 0},  // NE
    {0, 1},  // E
    {-1, 1}, // SE
    {-1, 0}, // SW
    {0, -1}  // W
};

void graph_generate(int m, struct graph_t *g, int (*in_hexagon)(int l, int c, int m)) {
  if (m < 2)
    return NULL;
  int num_vertices = 3 * (m * m) - 3 * m + 1;
  int num_edges = 9 * (m * m) - 15 * m + 6;
  // struct graph_t* g = graph_create(num_vertices);
  for (int l = 1 - m; l < m; ++l) {
    for (int c = 1 - m; c < m; ++c) {
      if (!in_hexagon(l, c, m))
        continue;
      int index = axial_to_index(l, c, m);
      for (int dir = 1; dir < 7; ++dir) {
        int l_voisin = l + directions[dir].l;
        int c_voisin = c + directions[dir].c;
        int index_voisin = axial_to_index(l_voisin, c_voisin, m);
        if (in_hexagon(l_voisin, c_voisin, m)) {
          gsl_spmatrix_uint_set(g->t, index, index_voisin, dir);
          ++g->num_edges;
        }
      }
    }
  }
  g->num_edges = g->num_edges / 2;
  assert(num_edges == g->num_edges);
  // return g;
}

struct graph_t *createGraph(int m, enum graph_type_t type) {
  struct graph_t *graph = (struct graph_t *)malloc(sizeof(struct graph_t));
  if (!graph) {
    perror("Failed to allocate memory for graph");
    return NULL;
  }
  // Calcul du nombre de sommets à partir du nombre m
  if (type == TRIANGULAR) {
    if (m < 2)
      return NULL;
    unsigned int n = 3 * (m * m) - 3 * m + 1;
  } else if (type == CYCLIC) {
    if (m < 3)
      return NULL;
    unsigned int n = 14 * m - 18;
  } else if (type == HOLEY) {
    if ((m < 6) || (m % 3 != 0))
      return NULL;
    unsigned int n = 2 * (m * m) * (1 / 3) + 18 * m - 48;
  }
  graph->num_vertices = n;
  graph->num_edges = 0;
  graph->type = type;
  graph->t = gsl_spmatrix_uint_alloc(n, n);

  // Construction des arêtes en fonction du type de graphe
  if (type == TRIANGULAR) {
    graph_generate(m, g, in_hexagon_T);
  } else if (type == CYCLIC) {
    graph_generate(m, g, in_hexagon_C);
  } else if (type == HOLEY) {
    graph_generate(m, g, in_hexagon_H);
  }

  return graph;
}

// Libération du graphe
void graph_free(struct graph_t *g) {
  if (!g)
    return;
  gsl_spmatrix_uint_free(g->t);
  free(g);
}

// Ajout d'une arête i -> j avec direction
void graph_add_edge(struct graph_t *g, int i, int j, int direction) {
  if (!g || i < 0 || j < 0 || i >= g->num_vertices || j >= g->num_vertices)
    return;
  if (direction < NW || direction > W)
    return;

  // On suppose que les arêtes sont symétriques dans le pavage
  gsl_spmatrix_uint_set(g->t, i, j, direction);
  gsl_spmatrix_uint_set(g->t, j, i,
                        opposite_dir(direction)); // direction opposée
  g->num_edges++;
}

// Affichage pour debug
void graph_print(const struct graph_t *g) {
  if (!g)
    return;
  printf("Graph: %d sommets, %d arêtes\n", g->num_vertices, g->num_edges);
  for (int i = 0; i < g->num_vertices; ++i) {
    for (int j = 0; j < g->num_vertices; ++j) {
      int dir = gsl_spmatrix_uint_get(g->t, i, j);
      if (dir != NO_EDGE) {
        printf("  %d --(%d)--> %d\n", i, dir, j);
      }
    }
  }
}

// Affichage formaté de la matrice d'adjacence
void graph_print_matrix(const struct graph_t *g) {
  if (!g)
    return;
  printf("Matrice d'adjacence (%d x %d) :\n", g->num_vertices, g->num_vertices);
  for (int i = 0; i < g->num_vertices; ++i) {
    printf("[ ");
    for (int j = 0; j < g->num_vertices; ++j) {
      int dir = gsl_spmatrix_uint_get(g->t, i, j);
      printf("%d ", dir);
    }
    printf("]\n");
  }
}

int main() {
  struct graph_t *g2 = createGraph(3, TRIANGULAR);
  graph_print_matrix(g2);
  graph_free(g2);
  return 0;
}
