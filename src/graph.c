#include "graph_functions.h"

// Définitions ANSI pour les couleurs
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define VIOLET "\033[1;35m"
#define RESET "\033[0m"

// Coordonnées axiales pour pavage hexagonal : (0, 0) en centre, (0, 1) vecteur
// déplacement East, (1, 0) vecteur déplacement North East, (1, -1) vecteur
// déplacement North West,
// struct axial_t;

// Conversion coordonnées (l, c) -> index dans le graphe
int axial_to_index(int l, int c, int m) {
  int i = m - l - 1;  // indice de la ligne dans la matrice equivalente à la
                      // notation graphe (son image)
  int j = m + c - 1;  // indice de la colonne dans la matrice equivalente à la
                      // notation graphe (son image)
  int count = 0;      // nombre d'elements dans la matrice non representes sur le
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
  l = l - l_origin;
  c = c - c_origin;
  return (abs(l) <= m - 1) && (abs(c) <= m - 1) && (abs(l + c) <= m - 1);
}

// Vérifie si (l, c) est bien dans l'hexagone de type cyclique
int in_hexagon_C(int l, int c, int m, int l_origin, int c_origin) {
  l = l - l_origin;
  c = c - c_origin;
  if (!((abs(l) <= m - 1) && (abs(c) <= m - 1) && (abs(l + c) <= m - 1)))
    return 0;
  int k = l + c;
  return ((l == m - 1) && (c <= 0 && c > -m)) || ((l == m - 2) && (c <= 1 && c > -m)) ||

         ((-l == m - 1) && (c >= 0 && c < m)) || ((-l == m - 2) && (c >= -1 && c < m)) ||

         ((c == m - 1) && (l <= 0 && l > -m)) || ((c == m - 2) && (l <= 1 && l > -m)) ||

         ((-c == m - 1) && (l >= 0 && l < m)) || ((-c == m - 2) && (l >= -1 && l < m)) ||

         (((abs(l) <= m - 1) && (abs(c) <= m - 1)) &&
          ((k == m - 1) || (k == m - 2) || (-k == m - 1) || (-k == m - 2)));
}

// Vérifie si (l, c) est bien dans l'hexagone de type trouée (HOLEY)
int in_hexagon_H(int l, int c, int m, int l_origin, int c_origin) {
  int m_prime     = m / 3 + 1;  // m du sous hexagone (il y en a 7)
  l               = l - l_origin;
  c               = c - c_origin;
  int deplacement = 2 * (m / 3) - 1;
  /*
  return (in_hexagon_C(l, c, m_prime, 0, 0)) ||
         (in_hexagon_C(l, c, m_prime, 0, deplacement)) ||
         (in_hexagon_C(l, c, m_prime, deplacement, 0)) ||
         (in_hexagon_C(l, c, m_prime, -deplacement, 0)) ||
         (in_hexagon_C(l, c, m_prime, 0, -deplacement)) ||
         (in_hexagon_C(l, c, m_prime, -deplacement, deplacement)) ||
         (in_hexagon_C(l, c, m_prime, deplacement, -deplacement));
         */

  int IN = (in_hexagon_C(l, c, m_prime, 0, 0)) + (in_hexagon_C(l, c, m_prime, 0, deplacement)) +
           (in_hexagon_C(l, c, m_prime, deplacement, 0)) +
           (in_hexagon_C(l, c, m_prime, -deplacement, 0)) +
           (in_hexagon_C(l, c, m_prime, 0, -deplacement)) +
           (in_hexagon_C(l, c, m_prime, -deplacement, deplacement)) +
           (in_hexagon_C(l, c, m_prime, deplacement, -deplacement));
  if (IN)
    return 1;
  else {
    if (!in_hexagon_T(l, c, m, 0, 0))
      return 0;
    IN = (in_hexagon_T(l, c, m_prime, 0, 0)) + (in_hexagon_T(l, c, m_prime, 0, deplacement)) +
         (in_hexagon_T(l, c, m_prime, deplacement, 0)) +
         (in_hexagon_T(l, c, m_prime, -deplacement, 0)) +
         (in_hexagon_T(l, c, m_prime, 0, -deplacement)) +
         (in_hexagon_T(l, c, m_prime, -deplacement, deplacement)) +
         (in_hexagon_T(l, c, m_prime, deplacement, -deplacement));
    return !IN;
  }
}

// Vecteurs de directions (en coord. axiales)
const struct axial_t directions[7] = {
    {0, 0},   // No edge
    {1, -1},  // NW
    {1, 0},   // NE
    {0, 1},   // E
    {-1, 1},  // SE
    {-1, 0},  // SW
    {0, -1}   // W
};

void graph_generate(int m, struct graph_t *g,
                    int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin)) {
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
        int l_voisin     = l + directions[dir].l;
        int c_voisin     = c + directions[dir].c;
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
  graph->num_vertices = 0;  // just for initialisation
  unsigned int n      = 3 * (m * m) - 3 * m + 1;
  graph->t            = gsl_spmatrix_uint_alloc(n, n);
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
    n = 2 * (m * m / 3) + 18 * m - 48;
  }
  graph->num_edges = 0;
  graph->type      = type;

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
  graph->num_objectives = 2;
  graph->objectives     = (vertex_t *)malloc(sizeof(vertex_t) * 2);
  graph->objectives[0]  = n / 2;  // Placer le premier objectif au centre du graphe (par exemple)
  int obj               = axial_to_index(m - 1, 0, m);
  graph->objectives[1] = obj;  // Placer le second objectif en haut à droite du graphe (par exemple)

  // Initialiser les positions de départ des joueurs
  graph->start[0] = 0;  // Premier joueur au sommet 0 //à changer au coordonnees axiales
  graph->start[1] = graph->num_vertices -
                    1;  // Deuxième joueur au dernier sommet //à changer au coordonnees axiales
  gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(graph->t, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(graph->t);
  graph->t = csr;
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
  }  // à modifier selon les coordonnees axiales

  // Affichage des objectifs
  printf("Objectives:\n");
  for (unsigned int i = 0; i < graph->num_objectives; i++) {
    printf("Objective %u: vertex %u\n", i, graph->objectives[i]);
  }
}
// Copie d'un graphe

void copy_graph(struct graph_t *dest, const struct graph_t *src) {
  // Copier les champs simples
  dest->type           = src->type;
  dest->num_vertices   = src->num_vertices;
  dest->num_edges      = src->num_edges;
  dest->num_objectives = src->num_objectives;

  memcpy(dest->start, src->start,
         sizeof(vertex_t) * NUM_PLAYERS);  // Copier start[]

  // Copier la matrice creuse (sparse matrix)
  dest->t                = gsl_spmatrix_uint_alloc(src->num_vertices, src->num_vertices);
  gsl_spmatrix_uint *csr = gsl_spmatrix_uint_compress(dest->t, GSL_SPMATRIX_CSR);
  gsl_spmatrix_uint_free(dest->t);
  dest->t = NULL;
  dest->t = csr;
  if (!dest->t) {
    fprintf(stderr, "Erreur allocation mémoire pour t\n");
    exit(1);
  }
  gsl_spmatrix_uint_memcpy(dest->t, src->t);  // Copie de la matrice creuse

  // Copier les objectifs (tableau dynamique)
  dest->objectives = malloc(src->num_objectives * sizeof(vertex_t));
  if (!dest->objectives) {
    fprintf(stderr, "Erreur allocation mémoire pour objectives\n");
    exit(1);
  }
  memcpy(dest->objectives, src->objectives,
         src->num_objectives * sizeof(vertex_t));  // Copie du tableau
}
// Libération du graphe
void graph_free(struct graph_t *g) {
  if (!g)
    return;
  if (g->t) {
    gsl_spmatrix_uint_free(g->t);
    g->t            = NULL;
    g->num_vertices = 2;
  }
  assert(g->t == NULL);
  if (g->objectives) {
    free(g->objectives);
    g->objectives = NULL;
  }
  assert(g->objectives == NULL);
  free(g);
  g = NULL;
}

// retourne 2 si il s'agit d'un objectif, 0 pour le joueur à la position
// start[0] et 1 pour le joueur en start[1]
int is_objective_or_player(int l, int c, int m, struct graph_t *g) {
  unsigned int ax = axial_to_index(l, c, m);
  if (ax == g->start[0])
    return 0;
  if (ax == g->start[1])
    return 1;
  for (unsigned int i = 0; i < g->num_objectives; ++i) {
    if (g->objectives[i] == ax)
      return 2;
  }
  return 3;
}

// impression du graphe avec les numeros d'indices
void print_hex_grid(struct graph_t *g) {
  int m                                                              = 0;
  int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin) = NULL;
  // Choix de la fonction selon le type
  switch (g->type) {
    case TRIANGULAR:
      // Retrouver m depuis le nombre de sommets
      m          = (int)((3 + sqrt(12 * g->num_vertices - 3)) / 6);
      in_hexagon = in_hexagon_T;
      break;
    case CYCLIC:
      // Retrouver m depuis le nombre de sommets
      m          = (int)((g->num_vertices + 18) / 12);
      in_hexagon = in_hexagon_C;
      break;
    case HOLEY:
      // Retrouver m depuis le nombre de sommets
      m          = (int)((-54 + sqrt(24 * g->num_vertices + 4068)) / 4);
      in_hexagon = in_hexagon_H;
      break;
    default:
      puts("Invalid graph type");
      return;
  }

  for (int l = m - 1; l > 0; --l) {
    for (int k = 0; k < l; ++k) {
      printf("   ");
    }
    for (int c = 1 - m; c < m; ++c) {
      if (in_hexagon_T(l, c, m, 0, 0)) {
        if (in_hexagon(l, c, m, 0, 0)) {
          if (is_objective_or_player(l, c, m, g) == 0) {
            int index = axial_to_index(l, c, m);
            printf(GREEN "%5d " RESET, index);
            continue;
          } else if (is_objective_or_player(l, c, m, g) == 1) {
            int index = axial_to_index(l, c, m);
            printf(BLUE "%5d " RESET, index);
            continue;
          } else if (is_objective_or_player(l, c, m, g) == 2) {
            int index = axial_to_index(l, c, m);
            printf(VIOLET "%5d " RESET, index);
            continue;
          }
          int index = axial_to_index(l, c, m);
          printf("%5d ", index);
        } else {
          printf("   " RED "E" RESET "  ");
        }
      }
    }
    printf("\n");
  }
  for (int l = 0; l > -m; --l) {
    for (int k = 0; k < abs(l); ++k) {
      printf("   ");
    }
    for (int c = 1 - m; c < m; ++c) {
      if (in_hexagon_T(l, c, m, 0, 0)) {
        if (in_hexagon(l, c, m, 0, 0)) {
          if (is_objective_or_player(l, c, m, g) == 0) {
            int index = axial_to_index(l, c, m);
            printf(GREEN "%5d " RESET, index);
            continue;
          } else if (is_objective_or_player(l, c, m, g) == 1) {
            int index = axial_to_index(l, c, m);
            printf(BLUE "%5d " RESET, index);
            continue;
          } else if (is_objective_or_player(l, c, m, g) == 2) {
            int index = axial_to_index(l, c, m);
            printf(VIOLET "%5d " RESET, index);
            continue;
          }
          int index = axial_to_index(l, c, m);
          printf("%5d ", index);
        } else {
          printf("   " RED "E" RESET "  ");
        }
      }
    }
    printf("\n");
  }
}


// fonction qui prend en entre l'indice du vertice et revoie la ligne et colone
// dans le graph
void index_to_axial(int index, int m, int *l, int *c) {
  for (int i = 1 - m; i < m; ++i) {
    for (int j = 1 - m; j < m; ++j) {
      if (in_hexagon_T(i, j, m, 0, 0) && (axial_to_index(i, j, m) == index)) {
        *l = i;
        *c = j;
        return;
      }
    }
  }
}


void graph_to_dot(const struct graph_t *g, const char *filename) {
  int m                                                              = 0;
  int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin) = NULL;

  switch (g->type) {
    case TRIANGULAR:
      m          = (int)((3 + sqrt(12 * g->num_vertices - 3)) / 6);
      in_hexagon = in_hexagon_T;
      break;
    case CYCLIC:
      m          = (int)((g->num_vertices + 18) / 12);
      in_hexagon = in_hexagon_C;
      break;
    case HOLEY:
      m          = (int)((-54 + sqrt(24 * g->num_vertices + 4068)) / 4);
      in_hexagon = in_hexagon_H;
      break;
    default:
      puts("Invalid graph type");
      return;
  }

  int l_origin = 0, c_origin = 0;

  FILE *f = fopen(filename, "w");
  if (!f) {
    perror("fopen");
    return;
  }

  fprintf(f, "graph G {\n");
  fprintf(f, "  node [shape=circle, fixedsize=true, width=0.4];\n");
  fprintf(f, "  graph [layout=neato, splines=true, overlap=false];\n");

  // Affichage des sommets
  for (size_t i = 0; i < g->num_vertices; ++i) {
    int l, c;
    index_to_axial(i, m, &l, &c);

    if (!in_hexagon(l, c, m, l_origin, c_origin))
      continue;

    double x = c * 1.0;
    double y = -l * sqrt(3.0) / 2.0 + (c % 2) * (sqrt(3.0) / 4.0);

    int is_player = 0;
    for (int p = 0; p < NUM_PLAYERS; ++p)
      if (g->start[p] == i)
        is_player = 1;

    if (is_player)
      fprintf(f, "  %zu [style=filled, fillcolor=lightblue, pos=\"%lf,%lf!\"];\n", i, x, y);
    else
      fprintf(f, "  %zu [pos=\"%lf,%lf!\"];\n", i, x, y);
  }

  // Affichage des arêtes
  for (size_t i = 0; i < g->num_vertices; ++i) {
    int l1, c1;
    index_to_axial(i, m, &l1, &c1);
    if (!in_hexagon(l1, c1, m, l_origin, c_origin))
      continue;

    for (size_t j = i + 1; j < g->num_vertices; ++j) {
      int l2, c2;
      index_to_axial(j, m, &l2, &c2);
      if (!in_hexagon(l2, c2, m, l_origin, c_origin))
        continue;

      unsigned int val = gsl_spmatrix_uint_get(g->t, i, j);
      if (val != 0) {
        if (val == 7)
          fprintf(f, "  %zu -- %zu [color=red, style=dashed];\n", i, j);
        else
          fprintf(f, "  %zu -- %zu;\n", i, j);
      }
    }
  }

  fprintf(f, "}\n");
  fclose(f);
}
