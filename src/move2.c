#include "graph.h"
#include "player.h"
#include "move.h"
#include "move2.h"
#include "graph_functions.h"
#include <stdlib.h>
#include <math.h>

//struct player (ajout last-position)
struct player_tt {
  unsigned int walls;
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
      
      int next_l = l + direc[dir].l;
      int next_c = c + direc[dir].c;
      
      if (!in_hexagon_T(l, c, m, 0, 0)) break;
      
      vertex_t from = axial_to_index(l, c, m);
      vertex_t to = axial_to_index(next_l, next_c, m);
      
      int exists = gsl_spmatrix_uint_get(g->t, from, to);
      if (exists == 0)
        break;// cas d'un mur
      vertex_t idx = axial_to_index(l, c, m);
      if (idx == target) return 1;

       // on continue a avancer dans cette direction
      l = next_l;
      c = next_c;

      // Verifie si une arete existe dans cette direction
      
    }
  }

  return 0;
}

#include "move.h"
#include "graph.h"
#include "player.h"



int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  if (p->walls <= 0) return 0;

  vertex_t fr1 = move.e[0].fr;
  vertex_t fr2 = move.e[1].fr;
  if (fr1 != fr2) return 0; // les deux arêtes ne partent pas du même sommet

  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  int dir1 = gsl_spmatrix_uint_get(g->t, fr1, to1);
  int dir2 = gsl_spmatrix_uint_get(g->t, fr1, to2);

  if (dir1 == 0 || dir2 == 0) return 0; // arêtes inexistantes

  int diff = abs(dir1 - dir2);
  if (diff != 1 && diff != 5) return 0; // pas consécutives

  return 1;
}



void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  vertex_t fr = move.e[0].fr;
  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  gsl_spmatrix_uint_set(g->t, fr, to1, 0);
  gsl_spmatrix_uint_set(g->t, to1, fr, 0);
  gsl_spmatrix_uint_set(g->t, fr, to2, 0);
  gsl_spmatrix_uint_set(g->t, to2, fr, 0);

  p->walls -= 1;
}

int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move) {
  if (move.t == MOVE) {
    // Deplacement du joueur
    vertex_t from = p->position;
    vertex_t to = move.m;

    if (!valid_move(g, p, to)) {
      return 0; // Deplacement invalide
    }

    // Mise à jour du joueur
    p->last_position = from;
    p->position = to;
    return 1;

  } else if (move.t == WALL) {
    // Pose d’un mur

    if (!valid_wall(g, p, move)) {
      return 0; // Mur invalide
    }

    // Appliquer le mur
    place_wall(g, p, move);
    return 1;
  }

  // Type de coup invalide
  return 0;
}


// fonction qui renvois tout les mouvements possibles dans un tableau passé en parametre
/*void availableMoves(struct move_t* moves[], struct graph_t *graph, int id_ofplayer, struct move_t* previous_move) {
    int nb_moves = 0;

    vertex_t current = get_player_position(id_ofplayer);
    vertex_t opponent = get_player_position( (id_ofplayer+1)%2);
    enum dir_t prev_dir = get_direction_from_move(previous_move);

    for (vertex_t i = 0 ; i<graph->num_vertices ; i++){
        if (i != opponent && is_connected(graph , current , i )) {
            moves[nb_moves++] = make_move_move(id_ofplayer, i);
        }
        if (i == opponent) {
            for (vertex_t j = 0 ; j<graph->num_vertices ;j++){ //each neighbor_of_opponent w in graph[opponent] 
                if (j != current && j != opponent &&is_connected(graph , opponent , j )) {
                    moves[nb_moves++] = make_move_move(id_ofplayer, j);
                }
            }
        }
    }
    moves[nb_moves] = NULL;
}*/





/*int main() {
  int m = 5;
  struct graph_t* g = createGraph(m, TRIANGULAR);

  struct player_tt p;
  p.last_position = axial_to_index(0, 0, m);   // déplacement précédent depuis (0,0)
  p.position = axial_to_index(0, 1, m);// jusqu’à (0,1) → vecteur = (0,1), direction EAST
  p.walls = 1;

  // On teste un mouvement en ligne droite (EAST) à distance 1, 2, 3
  vertex_t t1 = axial_to_index(0, 2, m); // 1 pas vers l'est
  vertex_t t2 = axial_to_index(0, 3, m); // 2 pas
  vertex_t t3 = axial_to_index(0, 4, m); // 3 pas
  vertex_t t4 = axial_to_index(1, 0, m); // dans une autre direction (NW ou NE)

  printf("Test déplacement vers (0,2) → %d\n", valid_move(g, &p, t1)); // attendu : 1
  printf("Test déplacement vers (0,3) → %d\n", valid_move(g, &p, t2)); // attendu : 1
  printf("Test déplacement vers (0,4) → %d\n", valid_move(g, &p, t3)); // attendu : 1
  printf("Test déplacement vers (1,0) → %d\n", valid_move(g, &p, t4)); // attendu : 1 ou 0 (dépend si 30° ou non)

 printf("=== Déplacement vers (0,2) ===\n");
  struct move_t move1 = {
    .t = MOVE,
    .c = BLACK,
    .m = axial_to_index(0, 2, m)
  };

  if (apply_move(g, &p, move1)) {
    printf("✅ Déplacement vers (0,2) réussi\n");
    printf("position du joueur %d: \n", p.position );
  }
  else {
    printf("❌ Déplacement vers (0,2) bloqué\n");
  }
  
  printf("=== Pose d’un mur entre (0,2)-(1,2) et (0,2)-(0,1) ===\n");
  struct move_t wall = {
    .t = WALL,
    .c = BLACK,
    .e = {
      { .fr = axial_to_index(0, 2, m), .to = axial_to_index(0, 1, m) },
      { .fr = axial_to_index(0, 2, m), .to = axial_to_index(-1, 2, m) }
    }
  };
  struct move_t wall2 = {
    .t = WALL,
    .c = BLACK,
    .e = {
      { .fr = axial_to_index(0, 2, m), .to = axial_to_index(1, 1, m) },
      { .fr = axial_to_index(0, 2, m), .to = axial_to_index(0, 1, m) }
    }
    };

  if (apply_move(g, &p, wall)) 
    printf("✅ Mur posé avec succès\n");
  } else {
    printf("❌ Pose du mur refusée\n");
  }

  printf("=== Tentative de retour vers (0,1) ===\n");
  struct move_t move2 = {
    .t = MOVE,
    .c = BLACK,
    .m = axial_to_index(0, 1, m)
  };

  if (valid_move(g, &p, move2.m)) {
    printf("✅ Retour vers (0,1) réussi\n");
  } else {
    printf("❌ Retour vers (0,1) bloqué par le mur\n");
  }

  graph_free(g);
  return 0;
}
*/
