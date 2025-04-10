#include "graph.h"
#include "player.h"
#include "move.h"
#include "graph_functions.h"
#include <stdlib.h>
#include <math.h>

//struct player (ajout last-position)
struct player_tt {
  vertex_t position;
  vertex_t last_position; // pour déduire la direction
};
//les differentes direction comme dans graph.c
const struct axial_t direc[7] = {
    {0, 0},  // No edge
    {1, -1}, // NW
    {1, 0},  // NE
    {0, 1},  // E
    {-1, 1}, // SE
    {-1, 0}, // SW
    {0, -1}  // W
};
// fonction qui prend en entre l'indice du vertice et revoie la ligne et colone dans le graph
void index_to_axial(int index, int m, int *l, int *c) {
  int count = 0;
  int row = 0;
  for (int i = 1 - m; i < m; ++i) {
    for (int j = 1 - m; j < m; ++j) {
      if (!in_hexagon_T(i, j, m, 0, 0)) continue; // si c'est dans le graph ou pas
      if (count == index) {
        *l = i;
        *c = j;
        return;
      }
      ++count;
    }
  }
}
//recois en entre ligne + colone et revoie la direction 
int direction_axial(int dl, int dc) {
  for (int d = 1; d < 7; ++d) {
    if (direc[d].l == dl && direc[d].c == dc)
      return d;
  }
  return 0;
}

// Renvoie vrai si le déplacement est possible selon les règles
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target) {
  if (p->position == target)
    return 0;

  int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3); // retrouve m d'apres le nombre de vertice
  int l0, c0, l1, c1;
  index_to_axial(p->last_position, m, &l0, &c0);//met dans l0 et c0 la ligne et colone de la position precedente du joueur
  index_to_axial(p->position, m, &l1, &c1);// met dans l1 et c1 la ligne et colone de la position actuel du joueur
  //pour savoir la direction du dernier mouvement
  int dl_prev = l1 - l0;
  int dc_prev = c1 - c0;

  int prev_dir = direction_axial(dl_prev, dc_prev); // la direction du dernier mouvement
  if (prev_dir == 0) return 0;//direction invalide ou illegal

  // explore les chemins dans chaque direction (1 à 6)
  for (int dir = 1; dir < 7; ++dir) {
    int max_dist = 1;
    if (dir == prev_dir)
      max_dist = 3;
    else if (abs(dir - prev_dir) == 1 || abs(dir - prev_dir) == 5) // pour les deplacement de 30 degre
      max_dist = 2;

    int l = l1, c = c1;
    for (int d = 1; d <= max_dist; ++d) {
      l += direc[dir].l;
      c += direc[dir].c;
      if (!in_hexagon_T(l, c, m, 0, 0)) break;

      vertex_t idx = axial_to_index(l, c, m);
      if (idx == target) return 1;

      // Verifie si une arete existe dans cette direction
      int exists = gsl_spmatrix_uint_get(g->t, axial_to_index(l - direc[dir].l, c - direc[dir].c, m), idx);
      if (exists == 0) break; // mur ?
    }
  }

  return 0;
}

int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move) {
  if (move.t != MOVE) {
    return 0; // Ce n'est pas un déplacement
  }

  vertex_t from = p->position;
  vertex_t to = move.m;

  // Vérification du coup
  if (!valid_move(g, p, to)) {
    return 0;
  }

  // Mise à jour de la position et du dernier déplacement
  p->last_position = from;
  p->position = to;

  return 1;
}



int main() {
  int m = 5;
  struct graph_t* g = createGraph(m, TRIANGULAR);

  struct player_tt p;
  p.last_position = axial_to_index(0, 0, m);   // déplacement précédent depuis (0,0)
  p.position = axial_to_index(0, 1, m);        // jusqu’à (0,1) → vecteur = (0,1), direction EAST

  // On teste un mouvement en ligne droite (EAST) à distance 1, 2, 3
  vertex_t t1 = axial_to_index(0, 2, m); // 1 pas vers l'est
  vertex_t t2 = axial_to_index(0, 3, m); // 2 pas
  vertex_t t3 = axial_to_index(0, 4, m); // 3 pas
  vertex_t t4 = axial_to_index(1, 0, m); // dans une autre direction (NW ou NE)

  printf("Test déplacement vers (0,2) → %d\n", valid_move(g, &p, t1)); // attendu : 1
  printf("Test déplacement vers (0,3) → %d\n", valid_move(g, &p, t2)); // attendu : 1
  printf("Test déplacement vers (0,4) → %d\n", valid_move(g, &p, t3)); // attendu : 1
  printf("Test déplacement vers (1,0) → %d\n", valid_move(g, &p, t4)); // attendu : 1 ou 0 (dépend si 30° ou non)

  struct move_t move = {
    .c = BLACK,
    .t = MOVE,
    .m = axial_to_index(2, 1, m)  // Je veux aller en (0,2)
  };
  struct move_t move2 = {
    .c = BLACK,
    .t = MOVE,
    .m = axial_to_index(0, 2, m)  // Je veux aller en (0,2)
  };
  printf("%d \n", move.m);
  printf("%d \n", p.position);
  if (apply_move(g, &p, move) && apply_move(g, &p, move2)) {
    printf("Déplacement fait\n");
    printf("%d \n", p.position);
  } else {
    printf("Déplacement invalide vers (0,2).\n");
  }
  
  
  
  graph_free(g);
  return 0;
}

