
#include "graph.h"
#include "player.h"
#include "move.h"
#include "move2.h"
#include "board.h"
#include "graph_functions.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>


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
  int row = 0;
  for (int i = 1 - m; i < m; ++i) {
    for (int j = 1 - m; j < m; ++j) {
      if (in_hexagon_T(i, j, m, 0, 0) && (axial_to_index(i,j,m)== index)) {
	
	*l = i;
	*c = j;
	return;
      }
      
    }
  }
}
/*void test_index_axial_inverse(int m) {
  int total = 0;
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
  printf("✔️  Tous les %d tests index <-> axial ont réussi pour m = %d\n", total, m);
  }*/
//recois en entre ligne + colone et revoie la direction 
int direction_axial(int dl, int dc) {
  for (int d = 1; d < 7; ++d) {
    if (direc[d].l == dl && direc[d].c == dc)
      return d;
  }
  return 0;
}

/*void test_direction_axial() {
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

  printf("✔️  Tous les tests de direction_axial sont passés avec succès.\n");
  }*/

// Renvoie vrai si le déplacement est possible selon les règles
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opponent_pos) {
  if (p->position == target || opponent_pos == target)
    return 0; // Interdit de rester sur place
  
  // Retrouver m
  int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
  
  // Convertir les index en coordonnées axiales
  int l0, c0, l1, c1;
  index_to_axial(p->last_position, m, &l0, &c0);
  index_to_axial(p->position, m, &l1, &c1);

  int dl_prev = l1 - l0;
  int dc_prev = c1 - c0;
  int prev_dir = direction_axial(dl_prev, dc_prev);

  if (prev_dir == 0)
    return 0; // Aucun déplacement précédent valide

  // Vérifier les directions possibles
  for (int dir = 1; dir < 7; ++dir) {
    int max_dist = 1;
    if (dir == prev_dir)
      max_dist = 3;
    else if ((dir == (prev_dir % 6) + 1) || (dir == (prev_dir + 4) % 6 + 1))
      max_dist = 2; // directions adjacentes (±30°)

    int l = l1;
    int c = c1;

    for (int d = 1; d <= max_dist; ++d) {
      l += direc[dir].l;
      c += direc[dir].c;

      if (!in_hexagon_T(l, c, m, 0, 0))
        break;

      vertex_t from = axial_to_index(l - direc[dir].l, c - direc[dir].c, m);
      vertex_t to = axial_to_index(l, c, m);

      int exists = gsl_spmatrix_uint_get(g->t, from, to);
      if (exists == 0 || exists == 7) // pas d’arête ou mur
        break;

      if (to == target)
        return 1; // Mouvement autorisé
    }
  }

  
  return 0; // aucun mouvement permis
}





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
  
  unsigned int* temp = gsl_spmatrix_uint_ptr(g->t, fr, to1);
  *temp = 7;
  unsigned int* temp1 = gsl_spmatrix_uint_ptr(g->t, to1, fr);
  *temp1 = 7;
  unsigned int* temp2 = gsl_spmatrix_uint_ptr(g->t, fr, to2);
  *temp2 = 7;
  unsigned int *temp3 = gsl_spmatrix_uint_ptr(g->t, to2, fr);
  *temp3 = 7;

  
  p->walls -= 1;
}

int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move, vertex_t opp) {
  if (move.t == MOVE) {
    // Deplacement du joueur
    vertex_t from = p->position;
    vertex_t to = move.m;
    
      if (!valid_move(g, p, to, opp)) {
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




int path_to_objective_exists(struct graph_t *g, vertex_t start, const vertex_t *objectives, size_t nb_obj) {
  int *visited = calloc(g->num_vertices, sizeof(int));
  if (!visited) return 0;

  vertex_t *queue = malloc(g->num_vertices * sizeof(vertex_t));
  if (!queue) {
    free(visited);
    return 0;
  }

  size_t front = 0, back = 0;
  visited[start] = 1;
  queue[back++] = start;

  while (front < back) {
    vertex_t u = queue[front++];

    // Objectif atteint ?
    for (size_t i = 0; i < nb_obj; ++i) {
      if (u == objectives[i]) {
        free(queue);
        free(visited);
        return 1;
      }
    }

    // Parcours de tous les éléments non nuls (en COO)
    for (size_t k = 0; k < g->t->nz; ++k) {
      vertex_t row = g->t->i[k];
      vertex_t col = g->t->p[k];
      unsigned int val = g->t->data[k];

      if (val == 7) continue; // mur → bloqué

      // Ajout du voisin si arête (u → v)
      if (row == u && !visited[col]) {
        visited[col] = 1;
        queue[back++] = col;
      }
    }
  }

  free(queue);
  free(visited);
  return 0;
}
//make_move_move
struct move_t* make_move_moove(enum player_color_t color, vertex_t dest) {
    struct move_t* move = malloc(sizeof(struct move_t));
    if (!move) {
        fprintf(stderr, "Erreur d'allocation mémoire pour le mouvement\n");
        exit(EXIT_FAILURE);
    }
    move->t = MOVE;
    move->c = color;
    move->m = dest;
    return move;
}


//

//function int  a void take an array and return this array full with all available moves that we could do and return the nuber of them 
int availableMoves(struct move_t moves[], struct graph_t *graph, struct player_tt *p ,vertex_t opponent) {
  int nb_moves = 0;
  enum dir_t prev_dir =gsl_spmatrix_uint_get(graph->t, p->last_position, p->position);

  for (vertex_t i = 0; i < graph->num_vertices; i++) {
    if (valid_move(graph, p, i, opponent)) {
      moves[nb_moves++] = *make_move_moove(p->c, i);
    }
  }
  


  return nb_moves;
}





struct move_t generate_random_valid_move(struct graph_t *g, struct player_tt *p, vertex_t opponent_pos) {
  int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
  int l, c;
  index_to_axial(p->position, m, &l, &c);

  int directions[6] = {1, 2, 3, 4, 5, 6};

  while (1) {
    // Mélanger les directions
    for (int i = 5; i > 0; --i) {
      int j = rand() % (i + 1);
      int tmp = directions[i];
      directions[i] = directions[j];
      directions[j] = tmp;
    }

    for (int i = 0; i < 6; ++i) {
      int dl = direc[directions[i]].l;
      int dc = direc[directions[i]].c;

      int l2 = l + dl;
      int c2 = c + dc;

      if (!in_hexagon_T(l2, c2, m, 0, 0)) continue;

      vertex_t dest = axial_to_index(l2, c2, m);

      if (valid_move(g, p, dest, opponent_pos)) {
        return (struct move_t){
          .t = MOVE,
          .c = p->c,
          .m = dest
        };
      }
    }

    // Facultatif : pour éviter boucle infinie si bloqué (sécurité)
    // Tu peux ajouter un compteur max essais si tu veux
  }
}






/*int main() {
  srand(time(NULL));
  int m = 5;
  struct graph_t* g = createGraph(m, TRIANGULAR);
  vertex_t opp = 7;

  struct player_tt p;
  p.last_position = 0;   // déplacement précédent depuis (0,0)
  p.position = 1;// jusqu’à (0,1) → vecteur = (0,1), direction EAST
  p.walls = 10;
  p.c = 0;

  // On teste un mouvement en ligne droite (EAST) à distance 1, 2, 3
 
  printf("position du joueur %d: \n", p.position );


  struct move_t move1 = generate_random_valid_move(g, &p, opp);
  if(apply_move(g, &p, move1,opp))
    printf("nouvelle position du joueur %d: \n", p.position );
  else
    printf("marche pas \n");

  struct move_t moves[1069];
  int nb = availableMoves(moves, g, &p, opp);
  
  printf("Nombre de déplacements possibles : %d\n", nb);
  for (int i = 0; i < nb; ++i) {
    printf(" %u ,",  moves[i].m);
  }
  printf("\n");

  
    
  graph_free(g);
  return 0;
}
*/


