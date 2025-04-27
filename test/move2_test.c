#include "graph_functions.h"
#include "move2.h"
#include "strategies.h"
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
  printf("test_index_axial_inverse passed.\n");
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
  printf("test_direction_axial passed.\n");

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
  printf("test_valid_wall passed.\n"); 
}

void test_dijkstra2() {
  int m = 5;
  struct graph_t* g = createGraph(9, TRIANGULAR);
  g->start[1] = 0;
  g->objectives[1] = 1;
  for (int i = 6; i < 217; ++i) {

  vertex_t start = 5;
  g->start[0] = start;
  vertex_t destination = i;
  g->objectives[0] = destination;
  vertex_t *path = malloc(g->num_vertices * sizeof(vertex_t));
  int length = shortest_path_length(g, start, destination, 0, path, start);
  /*
  print_hex_grid(g);
  printf("In order to go from vertex %d to vertex %d. ", start, destination);
  printf("Player TEST found this path using dijkstra with length %d :\n", length);
  for (vertex_t v = 0; path[v] != (unsigned int)-1; ++v) {
    printf("%d, ", path[v]);
  }
  printf("\n");
*/
  free(path);
  graph_free(g);
}

}

void test_valid_move() {
  

  struct graph_t *g = createGraph(5, TRIANGULAR);
  assert(g != NULL);

  struct player_tt p = {
    .walls = 10,
    .position = axial_to_index(0, 0, 5),
    .last_position = axial_to_index(0, -1, 5)
  };
  vertex_t opponent = axial_to_index(4, -4, 5); // NW

  // Test de mouvement dans la même direction (NE trois fois max)
  vertex_t t1 = axial_to_index(0, 1, 5);      
  vertex_t t2 = axial_to_index(0, 2, 5);      
  vertex_t t3 = axial_to_index(0, 3, 5);
  vertex_t t4 = axial_to_index(0, 4, 5);
  assert(valid_move(g, &p, t1, opponent));
  assert(valid_move(g, &p, t2, opponent));
  assert(valid_move(g, &p, t3, opponent));
  assert(!valid_move(g, &p, t4, opponent));// hors graphe ou arête absente

  // Test d’un angle à 30° (direction SE)
  vertex_t se1 = axial_to_index(1, 0, 5);     
  vertex_t se2 = axial_to_index(2, 0, 5);
  vertex_t se3 = axial_to_index(3, 0, 5);
  vertex_t se4 = axial_to_index(-1, 1, 5);     
  vertex_t se5 = axial_to_index(-2, 2, 5);
  vertex_t se6 = axial_to_index(-3, 3, 5);    
  assert(valid_move(g, &p, se1, opponent)); // déplacement latéral à distance 1 autorisé
  assert(valid_move(g, &p, se2, opponent));
  assert(!valid_move(g, &p, se3, opponent)); // trop loin
  assert(valid_move(g, &p, se4, opponent)); // déplacement latéral à distance 1 autorisé
  assert(valid_move(g, &p, se5, opponent));
  assert(!valid_move(g, &p, se6, opponent));

  // Test d’une direction non autorisée (ex: opposée)
  vertex_t invalid = axial_to_index(-2, 0, 5); // opposé à NE → devrait être refusé
  assert(!valid_move(g, &p, invalid, opponent));

  // Test interdit : case actuelle ou position adversaire
  assert(!valid_move(g, &p, p.position, opponent));
  assert(!valid_move(g, &p, opponent, opponent));

  // Blocage par un mur
  struct move_t mv = {
    .e = {
      {.fr = 30, .to = 22},
      {.fr = 30, .to = 31}
    }
  };
  place_wall(g, &p, mv); // bloqué par un mur
  assert(!valid_move(g, &p, 31, opponent));   // doit être interdit
  
  graph_free(g);
  printf("test_valid_move passed.\n");
}




       
