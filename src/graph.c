#include "graph_functions.h"

// Coordonnées axiales pour pavage hexagonal : (0, 0) en centre, (0, 1) vecteur
// déplacement East, (1, 0) vecteur déplacement North East, (1, -1) vecteur
// déplacement North West,
// struct axial_t;

// Conversion coordonnées (l, c) -> index dans le graphe
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
int in_hexagon_T(int l, int c, int m, int l_origin, int c_origin) {
  (void)l_origin;
  (void)c_origin;
  return (abs(l) <= m - 1) && (abs(c) <= m - 1) && (abs(l + c) <= m - 1);
}

// Vérifie si (l, c) est bien dans l'hexagone de type cyclique
int in_hexagon_C(int l, int c, int m, int l_origin, int c_origin) {
  l = l - l_origin;
  c = c - c_origin;
  if (!((abs(l) <= m - 1) && (abs(c) <= m - 1) && (abs(l + c) <= m - 1)))
    return 0;
  int k = l + c;
  return ((l == m - 1) && (c <= 0 && c > -m)) ||
         ((l == m - 2) && (c <= 1 && c > -m)) ||

         ((-l == m - 1) && (c >= 0 && c < m)) ||
         ((-l == m - 2) && (c >= -1 && c < m)) ||

         ((c == m - 1) && (l <= 0 && l > -m)) ||
         ((c == m - 2) && (l <= 1 && l > -m)) ||

         ((-c == m - 1) && (l >= 0 && l < m)) ||
         ((-c == m - 2) && (l >= -1 && l < m)) ||

         (((abs(l) <= m - 1) && (abs(c) <= m - 1)) &&
          ((k == m - 1) || (k == m - 2) || (-k == m - 1) || (-k == m - 2)));
}

// Vérifie si (l, c) est bien dans l'hexagone de type trouée (HOLEY)
int in_hexagon_H(int l, int c, int m, int l_origin, int c_origin) {
  // à faire ...
  int m_prime = m / 3; // m du sous hexagone (il y en a 7)
  l = l - l_origin;
  c = c - c_origin;
  return (in_hexagon_C(l, c, m_prime + 1, 0, 0)) ||
         (in_hexagon_C(l, c, m_prime + 1, 0, 2 * m_prime - 1)) ||
         (in_hexagon_C(l, c, m_prime + 1, 2 * m_prime - 1, 0)) ||
         (in_hexagon_C(l, c, m_prime + 1, -2 * m_prime + 1, 0)) ||
         (in_hexagon_C(l, c, m_prime + 1, 0, -2 * m_prime + 1)) ||
         (in_hexagon_C(l, c, m_prime + 1, -2 * m_prime + 1, 2 * m_prime - 1)) ||
         (in_hexagon_C(l, c, m_prime + 1, 2 * m_prime - 1, -2 * m_prime + 1));
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

void graph_generate(int m, struct graph_t *g,
                    int (*in_hexagon)(int l, int c, int m, int l_origin,
                                      int c_origin)) {
  if (m < 2) {
    perror("Failed to create graph; m < 2");
    return;
  }
  // struct graph_t* g = graph_create(num_vertices);
  for (int l = 1 - m; l < m; ++l) {
    for (int c = 1 - m; c < m; ++c) {
      if (!in_hexagon(l, c, m, 0, 0))
        continue;
      ++g->num_vertices;
      int index = axial_to_index(l, c, m);
      for (int dir = 1; dir < 7; ++dir) {
        int l_voisin = l + directions[dir].l;
        int c_voisin = c + directions[dir].c;
        int index_voisin = axial_to_index(l_voisin, c_voisin, m);
        if (in_hexagon(l_voisin, c_voisin, m, 0, 0)) {
          gsl_spmatrix_uint_set(g->t, index, index_voisin, dir);
          ++g->num_edges;
        }
      }
    }
  }
  g->num_edges = g->num_edges / 2;
}

// Cree un graphe de type "enum graph_type_t type" et de la variable m "int m"
struct graph_t *createGraph(int m, enum graph_type_t type) {
  struct graph_t *graph = (struct graph_t *)malloc(sizeof(struct graph_t));
  if (!graph) {
    perror("Failed to allocate memory for graph");
    return NULL;
  }
  unsigned int n = 3 * (m * m) - 3 * m + 1;
  graph->t = gsl_spmatrix_uint_alloc(n, n);
  // Calcul du nombre de sommets à partir du nombre m
  if (type == TRIANGULAR) {
    if (m < 2)
      return NULL;
  } else if (type == CYCLIC) {
    if (m < 3)
      return NULL;
    n = 14 * m - 18;
  } else if (type == HOLEY) {
    if ((m < 6) || (m % 3 != 0))
      return NULL;
    n = 2 * (m * m) * (1 / 3) + 18 * m - 48;
  }
  graph->num_edges = 0;
  graph->type = type;

  // Construction des arêtes en fonction du type de graphe
  if (type == TRIANGULAR) {
    graph_generate(m, graph, in_hexagon_T);
  } else if (type == CYCLIC) {
    graph_generate(m, graph, in_hexagon_C);
  } else if (type == HOLEY) {
    graph_generate(m, graph, in_hexagon_H);
  }

  // Initialiser les objectifs et les positions des joueurs
  // à modifier
  graph->num_objectives = 1;
  graph->objectives = (vertex_t *)malloc(sizeof(vertex_t));
  graph->objectives[0] =
      n / 2; // Placer l'objectif au centre du graphe (par exemple)

  // Initialiser les positions de départ des joueurs
  graph->start[0] =
      0; // Premier joueur au sommet 0 //à changer au coordonnees axiales
  graph->start[1] =
      n -
      1; // Deuxième joueur au dernier sommet //à changer au coordonnees axiales

  return graph;
}

// Affichage formaté de la matrice d'adjacence
void graph_print_matrix(const struct graph_t *g) {
  if (!g)
    return;
  printf("Matrice d'adjacence (%d x %d) :\n", g->num_vertices, g->num_vertices);
  for (unsigned int i = 0; i < g->num_vertices; ++i) {
    printf("[ ");
    for (unsigned int j = 0; j < g->num_vertices; ++j) {
      int dir = gsl_spmatrix_uint_get(g->t, i, j);
      printf("%d ", dir);
    }
    printf("]\n");
  }
}

// Affichage pour debug
void graph_print(struct graph_t *graph) {
  if (!graph)
    return;
  printf("Graph Type: %d\n", graph->type);
  printf("Number of vertices: %u\n", graph->num_vertices);
  printf("Number of edges: %u\n", graph->num_edges);

  // Affichage de la matrice d'adjacence
  graph_print_matrix(graph);

  // Affichage des positions de départ des joueurs
  printf("Starting positions:\n");
  for (int i = 0; i < NUM_PLAYERS; i++) {
    printf("Player %d starts at vertex %u\n", i, graph->start[i]);
  } // à modifier selon les coordonnees axiales

  // Affichage des objectifs
  printf("Objectives:\n");
  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    printf("Objective %u: vertex %u\n", i, graph->objectives[i]);
  }
}

// Libération du graphe
void graph_free(struct graph_t *g) {
  if (!g)
    return;
  gsl_spmatrix_uint_free(g->t);
  if (g->objectives)
    free(g->objectives);
  free(g);
}