
#include "move2.h"
#include "board.h"
#include "graph.h"
#include "graph_functions.h"
#include "move.h"
#include "player.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// les differentes direction comme dans graph.c
const struct axial_t direc[7] = {
    {0, 0},  // No edge
    {1, -1}, // NW
    {1, 0},  // NE
    {0, 1},  // E
    {-1, 1}, // SE
    {-1, 0}, // SW
    {0, -1}  // W
};
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
// recois en entre ligne + colone et revoie la direction
int direction_axial(int dl, int dc) {
  for (int d = 1; d < 7; ++d) {
    if (direc[d].l == dl && direc[d].c == dc)
      return d;
  }
  return 0;
}

// Renvoie vrai si le déplacement est possible selon les règles
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target,
               vertex_t opponent_pos) {

  // printf("DEBUG VALID_MOVE: from %u (last %u) to %u, opponent at %u\n",
  // p->position, p->last_position, target, opponent_pos);

  if (p->position == target || opponent_pos == target)
    return 0; // Interdit de rester sur place

  // Retrouver m
  // int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
  int m = 0;
  int (*in_hexagon)(int l, int c, int m, int l_origin, int c_origin) = NULL;
  // Choix de la fonction selon le type
  switch (g->type) {
  case TRIANGULAR:
    // Retrouver m depuis le nombre de sommets
    m = (int)((3 + sqrt(12 * g->num_vertices - 3)) / 6);
    in_hexagon = in_hexagon_T;
    break;
  case CYCLIC:
    // Retrouver m depuis le nombre de sommets
    m = (int)((g->num_vertices + 18) / 12);
    in_hexagon = in_hexagon_C;
    break;
  case HOLEY:
    // Retrouver m depuis le nombre de sommets
    m = (int)((-54 + sqrt(24 * g->num_vertices + 4068)) / 4);
    in_hexagon = in_hexagon_H;
    break;
  default:
    puts("Invalid graph type");
    return 0;
  }
  // Convertir les index en coordonnées axiales
  int l0 = 0;
  int c0 = 0;
  int l1 = 0;
  int c1 = 0;
  index_to_axial(p->last_position, m, &l0, &c0);
  index_to_axial(p->position, m, &l1, &c1);

  int dl_prev = l1 - l0;
  int dc_prev = c1 - c0;

  // Normalisation du vecteur direction afin qu'il soit reconnu par
  // direction_axial par exemple dir (3, 0) devient (1, 0)
  int max_abs = fmax(abs(dl_prev), abs(dc_prev));
  if (max_abs != 0) {
    dl_prev /= max_abs;
    dc_prev /= max_abs;
  }
  int prev_dir = direction_axial(dl_prev, dc_prev);
  if (dl_prev == 0 && dc_prev == 0){
    prev_dir = 0; // Aucun déplacement précédent valide

  }
  /*
    if (prev_dir == 0)
      return 0; // Aucun déplacement précédent valide
  */
  // Vérifier les directions possibles
  for (int dir = 1; dir < 7; ++dir) {

    int max_dist = 1;
    if (prev_dir==0){
      max_dist = 1; // Premier mouvement
    }
    else {
    if (dir == prev_dir)
      max_dist = 3;
    else if ((dir == (prev_dir % 6) + 1) || (dir == (prev_dir + 4) % 6 + 1))
      max_dist = 2; // directions adjacentes (±30°)
    }
    int l = l1;
    int c = c1;

    for (int d = 1; d <= max_dist; ++d) {
      l += direc[dir].l;
      c += direc[dir].c;

      if (!in_hexagon(l, c, m, 0, 0))
        break;

      vertex_t from = axial_to_index(l - direc[dir].l, c - direc[dir].c, m);
      vertex_t to = axial_to_index(l, c, m);

      int exists = gsl_spmatrix_uint_get(g->t, from, to);
      if (exists == 0 || exists == 7 ||
          ((from == opponent_pos) && d == 3)) // pas d’arête ou mur
        break;

      if (to == target) {
        return d; // Mouvement autorisé et on retourne la distance du saut
      }
    }
  }

  return 0; // aucun mouvement permis
}

int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  if (p->walls <= 0)
    return 0;

  vertex_t fr1 = move.e[0].fr;
  vertex_t fr2 = move.e[1].fr;
  if (fr1 != fr2)
    return 0; // les deux arêtes ne partent pas du même sommet

  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  int dir1 = gsl_spmatrix_uint_get(g->t, fr1, to1);
  int dir2 = gsl_spmatrix_uint_get(g->t, fr1, to2);

  if (dir1 == 0 || dir2 == 0)
    return 0; // arêtes inexistantes

  int diff = abs(dir1 - dir2);
  if (diff != 1 && diff != 5)
    return 0; // pas consécutives

  return 1;
}

void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  vertex_t fr = move.e[0].fr;
  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  unsigned int *temp = gsl_spmatrix_uint_ptr(g->t, fr, to1);
  *temp = 7;
  unsigned int *temp1 = gsl_spmatrix_uint_ptr(g->t, to1, fr);
  *temp1 = 7;
  unsigned int *temp2 = gsl_spmatrix_uint_ptr(g->t, fr, to2);
  *temp2 = 7;
  unsigned int *temp3 = gsl_spmatrix_uint_ptr(g->t, to2, fr);
  *temp3 = 7;

  p->walls -= 1;
}

int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move,
               vertex_t opp) {
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

int path_to_objective_exists(struct graph_t *g, vertex_t start,
                             const vertex_t *objectives, size_t nb_obj) {
  int *visited = calloc(g->num_vertices, sizeof(int));
  if (!visited)
    return 0;

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

      if (val == 7)
        continue; // mur → bloqué

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
// make_move_move
struct move_t make_move_moove(enum player_color_t color, vertex_t dest) {
  struct move_t move;
  move.t = MOVE;
  move.c = color;
  move.m = dest;
  return move;
}

struct move_t *make_wall_move(enum player_color_t color, vertex_t fr1,
                              vertex_t to1, vertex_t fr2, vertex_t to2) {
  struct move_t *move = malloc(sizeof(struct move_t));
  if (!move) {
    fprintf(stderr, "Erreur d'allocation mémoire pour le mur\n");
    exit(EXIT_FAILURE);
  }
  move->t = WALL;
  move->c = color;
  move->e[0].fr = fr1;
  move->e[0].to = to1;
  move->e[1].fr = fr2;
  move->e[1].to = to2;
  return move;
}

//

// function int  a void take an array and return this array full with all
// available moves that we could do and return the nuber of them
int availableMovess(struct move_t moves[], struct graph_t *graph,
                    struct player_tt *p, vertex_t opponent) {
  int nb_moves = 0;
  for (vertex_t i = 0; i < graph->num_vertices; i++) {
    if (valid_move(graph, p, i, opponent) && i != p->position) {
      moves[nb_moves++] = make_move_moove(p->c, i);
    }
  }

  return nb_moves;
}

/*int availableWalls(struct move_t moves[], struct graph_t *graph, struct
player_tt *p ,vertex_t opponent) { int nb_moves = 0; (void) opponent; for
(vertex_t i = 0; i < graph->num_vertices; i++) { for (vertex_t j = 0; j <
graph->num_vertices; j++) { for (vertex_t z = 0; z < graph->num_vertices; z++) {
        if (i == j || i == z || j == z) continue; // éviter les murs en
diagonale

      struct move_t wall_p = {
        .t = WALL,
        .c = p->c,
        .e[0].fr = i,
        .e[0].to = j,
        .e[1].fr = i,
        .e[1].to = z
      };
      if (valid_wall(graph, p, wall_p)) {

        struct move_t move = *make_wall_move(p->c, i, j);
        move.m = p->position;
        moves[nb_moves++] = move;
      }
    }
    }
  }
  return nb_moves;
}*/

int availableMoves(struct move_t moves[], struct graph_t *graph,
                   struct player_tt *p, vertex_t opponent) {
  int nb_moves = 0;
  nb_moves += availableMovess(moves, graph, p, opponent);
  //  nb_moves += availableWalls(moves + nb_moves, graph, p, opponent);
  return nb_moves;
}

struct move_t generate_random_valid_move(struct graph_t *g, struct player_tt *p,
                                         vertex_t opponent_pos) {
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

      if (!in_hexagon_T(l2, c2, m, 0, 0))
        continue;

      vertex_t dest = axial_to_index(l2, c2, m);

      if (valid_move(g, p, dest, opponent_pos)) {
        return (struct move_t){.t = MOVE, .c = p->c, .m = dest};
      }
    }
  }
}
/*

int main(){
  //test valid_move
  struct graph_t *g = createGraph(6, TRIANGULAR);
  g->num_objectives = 1 ; 
  g->objectives[0] = 29;
  struct player_tt p;
  p.position = 50;
  p.last_position = 50;
  p.c = 0;
  vertex_t opponent_pos = 0;
  vertex_t target = 29;
  int *path = malloc(g->num_vertices * sizeof(int));
  int dist = shortest_path_length(g, p.position, target, opponent_pos, path,
                                  p.last_position);
                          
  printf("Shortest path length from %u to %u: %d\n", p.position, target, dist);
  printf("Path: ");
  for (int i = 0; i < dist+1; ++i) {
    printf("%u ", path[i]);
  }
  printf("\n");
  int result = valid_move(g, &p, target, opponent_pos);
  printf("Valid move from %u to %u: %d\n", p.position, target, result);
  p.position = 6;
  p.last_position = 0;
  p.c = 0;
  
  int result2 = valid_move(g, &p, 30, 21);
  printf("Valid move from %u to %u: %d\n", p.position, 30, result2);
}
*/