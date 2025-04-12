#include "graph.h"
#include "player.h"
#include "move.h"
#include "move2.h"
#include "graph_functions.h"
#include <stdlib.h>
#include <math.h>


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
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opponent_pos) {
  if (p->position == target)
    return 0; // pas de déplacement vers soi-même

  // Retrouver m depuis le nombre de sommets
  int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);

  // Coordonnées axiales du dernier déplacement
  int l0, c0, l1, c1;
  index_to_axial(p->last_position, m, &l0, &c0);
  index_to_axial(p->position, m, &l1, &c1);

  int dl_prev = l1 - l0;
  int dc_prev = c1 - c0;

  int prev_dir = direction_axial(dl_prev, dc_prev);
  if (prev_dir == 0)
    return 0; // aucun déplacement précédent valable

  // 🔁 Tentatives de déplacement classiques selon les règles
  for (int dir = 1; dir < 7; ++dir) {
    int max_dist = 1;
    if (dir == prev_dir)
      max_dist = 3;
    else if (abs(dir - prev_dir) == 1 || abs(dir - prev_dir) == 5)
      max_dist = 2;

    int l = l1, c = c1;

    for (int d = 1; d <= max_dist; ++d) {
      int next_l = l + direc[dir].l;
      int next_c = c + direc[dir].c;

      if (!in_hexagon_T(next_l, next_c, m, 0, 0))
        break;

      vertex_t from = axial_to_index(l, c, m);
      vertex_t to = axial_to_index(next_l, next_c, m);

      int exists = gsl_spmatrix_uint_get(g->t, from, to);
      if (exists == 7)
        break;

      if (to == target)
        return 1;

      // avancer
      l = next_l;
      c = next_c;
    }
  }

  // ♟️ Tentative de saut par-dessus l’adversaire (sans graph_neighbors)
  for (int dir = 1; dir < 7; ++dir) {
    int l_adj = l1 + direc[dir].l;
    int c_adj = c1 + direc[dir].c;

    if (!in_hexagon_T(l_adj, c_adj, m, 0, 0))
      continue;

    vertex_t adj_idx = axial_to_index(l_adj, c_adj, m);

    if (adj_idx != opponent_pos)
      continue; // ce n’est pas l’adversaire

    // L’adversaire est adjacent → regarder ses cases voisines
    for (int d = 1; d < 7; ++d) {
      int l2 = l_adj + direc[d].l;
      int c2 = c_adj + direc[d].c;

      if (!in_hexagon_T(l2, c2, m, 0, 0))
        continue;

      vertex_t dest = axial_to_index(l2, c2, m);

      if (dest == p->position)
        continue;

      if (dest == target) {
        int exists = gsl_spmatrix_uint_get(g->t, adj_idx, dest);
        if (exists != 0)
          return 1; // saut valide
      }
    }
  }
  
  return 0; // aucun mouvement permis
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
  
  unsigned int* temp = gsl_spmatrix_uint_ptr(g->t, fr, to1);
  *temp = 7;
  temp = gsl_spmatrix_uint_ptr(g->t, to1, fr);
  *temp = 7;
  temp = gsl_spmatrix_uint_ptr(g->t, fr, to2);
  *temp = 7;
  temp = gsl_spmatrix_uint_ptr(g->t, to2, fr);
  *temp = 7;

  
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



/*vertex_t get_player_position(int id_ofplayer) {
    return players[id_ofplayer].position;
} 
vertex_t get_opponent_position(int id_ofplayer) {
    return players[(id_ofplayer + 1) % NUM_PLAYERS].position;
}
// fonction qui renvoie la direction du dernier mouvement
enum dir_t get_direction_from_move(struct move_t* move) {
    int l0, c0, l1, c1;
    index_to_axial(move->m, 5, &l0, &c0);
    index_to_axial(move->last_position, 5, &l1, &c1);
    int dl = l1 - l0;
    int dc = c1 - c0;
    return direction_axial(dl, dc);
    }*/






// fonction qui renvois tout les mouvements possibles dans un tableau passé en parametre


//make_move_move
/*
struct move_t* make_move_move(enum player_color_t color, vertex_t dest) {
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



int  availableMoves(struct move_t* moves[], struct graph_t *graph, struct move_t* previous_move ,vertex_t current ,vertex_t opponent ) {
    int nb_moves = 0;
    enum dir_t prev_dir = get_direction_from_move(previous_move);

    for (vertex_t i = 0 ; i<graph->num_vertices ; i++){
        if (valid_move(graph , current , i )) {
            moves[nb_moves++] = make_move_move  ((previous_move->c+1)%2, i);
        }
    moves[nb_moves] = NULL;
    return nb_moves;
}
*/

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

    for (vertex_t v = 0; v < g->num_vertices; ++v) {
      if (!visited[v] && gsl_spmatrix_uint_get(g->t, u, v) != 7) {
        visited[v] = 1;
        queue[back++] = v;
      }
    }
  }

  // Aucun objectif atteint
  free(queue);
  free(visited);
  return 0;
}




/*int main() {
  int m = 5;
  struct graph_t* g = createGraph(m, TRIANGULAR);
  vertex_t opp = 7; 

  struct player_tt p;
  p.last_position = axial_to_index(0, 0, m);   // déplacement précédent depuis (0,0)
  p.position = axial_to_index(0, 1, m);// jusqu’à (0,1) → vecteur = (0,1), direction EAST
  p.walls = 1;

  // On teste un mouvement en ligne droite (EAST) à distance 1, 2, 3
  vertex_t t1 = axial_to_index(0, 2, m); // 1 pas vers l'est
  vertex_t t2 = axial_to_index(0, 3, m); // 2 pas
  vertex_t t3 = axial_to_index(0, 4, m); // 3 pas
  vertex_t t4 = axial_to_index(1, 0, m); // dans une autre direction (NW ou NE)

  printf("Test déplacement vers (0,2) → %d\n", valid_move(g, &p, t1,opp)); // attendu : 1
  printf("Test déplacement vers (0,3) → %d\n", valid_move(g, &p, t2,opp)); // attendu : 1
  printf("Test déplacement vers (0,4) → %d\n", valid_move(g, &p, t3,opp)); // attendu : 1
  printf("Test déplacement vers (1,0) → %d\n", valid_move(g, &p, t4,opp)); // attendu : 1 ou 0 (dépend si 30° ou non)

 printf("=== Déplacement vers (0,2) ===\n");
  struct move_t move1 = {
    .t = MOVE,
    .c = BLACK,
    .m = axial_to_index(0, 2, m)
  };

  if (apply_move(g, &p, move1,opp)) {
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
  struct move_t wall1 = {
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

  if (apply_move(g, &p, wall,opp)) 
    printf("✅ Mur posé avec succès\n");
  else {
    printf("❌ Pose du mur refusée\n");
  }

  printf("=== Tentative de retour vers (0,1) ===\n");
  struct move_t move2 = {
    .t = MOVE,
    .c = BLACK,
    .m = axial_to_index(0, 1, m)
  };

  if (valid_move(g, &p, move2.m,opp)) {
    printf("✅ Retour vers (0,1) réussi\n");
  } else {
    printf("❌ Retour vers (0,1) bloqué par le mur\n");
  }

  graph_free(g);
  return 0;
}
*/
