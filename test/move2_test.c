#include "graph_functions.h"
#include "move2.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

void test_index_axial_inverse() {
  int total = 0;
  for (int m = 2; m<10;m++){
    for (int l = 1 - m; l < m; ++l) {
      for (int c = 1 - m; c < m; ++c) {
	if (in_hexagon_T(l, c, m, 0, 0)) {
	  int index = axial_to_index(l, c, m);
	  int l2, c2;
	  index_to_axial(index, m, &l2, &c2);
	  // Vérification : aller-retour (l,c) -> index -> (l2,c2)
	  assert(l == l2);
	  assert(c == c2);
	  ++total;
	}
      }
    }
  }
}
void test_direction_axial() {
  // Tests valides
  assert(direction_axial(1, -1) == 1);   // NW
  assert(direction_axial(1, 0) == 2);    // NE
  assert(direction_axial(0, 1) == 3);    // E
  assert(direction_axial(-1, 1) == 4);   // SE
  assert(direction_axial(-1, 0) == 5);   // SW
  assert(direction_axial(0, -1) == 6);   // W

  // Cas invalide : aucun vecteur de direction
  assert(direction_axial(0, 0) == 0);
  assert(direction_axial(2, -1) == 0);
  assert(direction_axial(-1, -1) == 0);

}


void test_valid_wall() {

  struct graph_t *g = createGraph(5, TRIANGULAR);
  assert(g != NULL);

  struct player_tt p = { .walls = 50 };

  vertex_t center = axial_to_index(0, 0, 5);
  vertex_t e     = axial_to_index(0, 1, 5);
  vertex_t ne    = axial_to_index(1, 0, 5);
  vertex_t nw    = axial_to_index(1, -1, 5);

  // Cas valide : NW + NE = directions 1 et 2 → consécutives
  struct move_t m1 = {
    .e = {
      {.fr = center, .to = nw},
      {.fr = center, .to = ne}
    }
  };
  assert(valid_wall(g, &p, m1) );

  // Cas invalide : arêtes non consécutives (ex: NW et E = dir 1 et 3 → diff = 2)
  struct move_t m2 = {
    .e = {
      {.fr = center, .to = nw},
      {.fr = center, .to = e}
    }
  };
  assert(!valid_wall(g, &p, m2));

  // Cas invalide : une des arêtes inexistantes
  struct move_t m3 = {
    .e = {
      {.fr = center, .to = 1}, // sommet inexistant
      {.fr = center, .to = e}
    }
  };
  assert(!valid_wall(g, &p, m3) );

  // Cas invalide : murs épuisés
  p.walls = 0;
  assert(!valid_wall(g, &p, m1) );

  graph_free(g);
  
}



       
