
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
    {0, 0},   // No edge
    {1, -1},  // NW
    {1, 0},   // NE
    {0, 1},   // E
    {-1, 1},  // SE
    {-1, 0},  // SW
    {0, -1}   // W
};
// recois en entre ligne + colone et revoie la direction
int direction_axial(int dl, int dc) {
  for (int d = 1; d < 7; ++d) {
    if (direc[d].l == dl && direc[d].c == dc)
      return d;
  }
  return 0;
}

// Renvoie vrai si le déplacement est possible selon les règles
int valid_move(struct graph_t *g, struct player_tt *p, vertex_t target, vertex_t opponent_pos) {
  // printf("DEBUG VALID_MOVE: from %u (last %u) to %u, opponent at %u\n",
  // p->position, p->last_position, target, opponent_pos);

  if (p->position == target || opponent_pos == target)
    return 0;  // Interdit de rester sur place

  // Retrouver m
  // int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);

  int               m          = 0;
  in_hexagon_func_t in_hexagon = NULL;

  // Choix de la fonction selon le type
  resolve_graph_type_or_default(g, &m, &in_hexagon);

  // Convertir les index en coordonnées axiales
  int l0 = 0;
  int c0 = 0;
  int l1 = 0;
  int c1 = 0;
  index_to_axial(p->last_position, m, &l0, &c0, g->type);
  index_to_axial(p->position, m, &l1, &c1, g->type);

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
  if (dl_prev == 0 && dc_prev == 0) {
    prev_dir = 0;  // Aucun déplacement précédent valide
  }
  /*
    if (prev_dir == 0)
      return 0; // Aucun déplacement précédent valide
  */
  // Vérifier les directions possibles
  for (int dir = 1; dir < 7; ++dir) {
    int max_dist = 1;
    if (prev_dir == 0) {
      max_dist = 1;  // Premier mouvement
    } else {
      if (dir == prev_dir)
        max_dist = 3;
      else if ((dir == (prev_dir % 6) + 1) || (dir == (prev_dir + 4) % 6 + 1))
        max_dist = 2;  // directions adjacentes (±30°)
    }
    int l = l1;
    int c = c1;

    vertex_t route[4] = {0};  // stock à chaque fois le parcours du joueur lors du saut
    int      count    = 0;

    for (int d = 1; d <= max_dist; ++d) {
      l += direc[dir].l;
      c += direc[dir].c;

      if (!in_hexagon(l, c, m, 0, 0))
        break;

      vertex_t from  = axial_to_index(l - direc[dir].l, c - direc[dir].c, m, g->type);
      route[count++] = from;
      vertex_t to    = axial_to_index(l, c, m, g->type);
      route[count++] = to;

      int exists = gsl_spmatrix_uint_get(g->t, from, to);
      if (exists == 0 || exists == 7)  // pas d’arête ou mur
        break;
      if (max_dist == 3 && d == 3) {
        for (int i = 0; i < 4; ++i) {  // on verifie si l'adversaire est dans l'une des positions
                                       // sur lesquelles il veut sauter
          // printf("%d, ", route[i]);
          if (route[i] == opponent_pos) {
            return 0;
          }
        }
      }
      // printf("\n");
      // route[0] = 0;
      // route[1] = 0;
      // route[2] = 0;
      // route[3] = 0;
      if (to == target) {
        return d;  // Mouvement autorisé et on retourne la distance du saut
      }
    }
  }

  return 0;  // aucun mouvement permis
}

int valid_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  if (p->walls <= 0) {
    printf("Refusé : plus de murs\n");
    return 0;
  }

  vertex_t fr1 = move.e[0].fr;
  vertex_t fr2 = move.e[1].fr;
  if (fr1 != fr2) {
    printf("Refusé : les arêtes ne partent pas du même sommet (%u != %u)\n", fr1, fr2);
    return 0;
  }

  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  int dir1 = gsl_spmatrix_uint_get(g->t, fr1, to1);
  int dir2 = gsl_spmatrix_uint_get(g->t, fr2, to2);

  if (dir1 == 0 || dir2 == 0) {
    printf("Refusé : arête absente (%u->%u ou %u->%u)\n", fr1, to1, fr1, to2);
    return 0;
  }

  if (dir1 == dir2) {
    printf("Refusé : mêmes directions (%d == %d)\n", dir1, dir2);
    return 0;
  }

  int diff = abs(dir1 - dir2);
  if (diff != 1 && diff != 5) {
    printf(
        "Refusé : directions %d et %d ne sont pas consécutives fr1=%d et fr2=%d to1=%d to2=%det \n",
        dir1, dir2, fr1, fr2, to1, to2);
    return 0;
  }

  return 1;
}

void place_wall(struct graph_t *g, struct player_tt *p, struct move_t move) {
  vertex_t fr  = move.e[0].fr;
  vertex_t to1 = move.e[0].to;
  vertex_t to2 = move.e[1].to;

  unsigned int *temp  = gsl_spmatrix_uint_ptr(g->t, fr, to1);
  *temp               = 7;
  unsigned int *temp1 = gsl_spmatrix_uint_ptr(g->t, to1, fr);
  *temp1              = 7;
  unsigned int *temp2 = gsl_spmatrix_uint_ptr(g->t, fr, to2);
  *temp2              = 7;
  unsigned int *temp3 = gsl_spmatrix_uint_ptr(g->t, to2, fr);
  *temp3              = 7;

  p->walls -= 1;
}

int apply_move(struct graph_t *g, struct player_tt *p, struct move_t move, vertex_t opp) {
  if (move.t == MOVE) {
    // Deplacement du joueur
    vertex_t from = p->position;
    vertex_t to   = move.m;

    if (!valid_move(g, p, to, opp)) {
      return 0;  // Deplacement invalide
    }

    // Mise à jour du joueur
    p->last_position = from;
    p->position      = to;
    return 1;

  } else if (move.t == WALL) {
    // Pose d’un mur

    if (!valid_wall(g, p, move)) {
      return 0;  // Mur invalide
    }

    // Appliquer le mur
    place_wall(g, p, move);
    return 1;
  }

  // Type de coup invalide
  return 0;
}

int path_to_objective_exists(struct graph_t *g, vertex_t start, const vertex_t *objectives,
                             size_t nb_obj) {
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
  queue[back++]  = start;

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
      vertex_t     row = g->t->i[k];
      vertex_t     col = g->t->p[k];
      unsigned int val = g->t->data[k];

      if (val == 7)
        continue;  // mur → bloqué

      // Ajout du voisin si arête (u → v)
      if (row == u && !visited[col]) {
        visited[col]  = 1;
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

struct move_t *make_wall_move(enum player_color_t color, vertex_t fr1, vertex_t to1, vertex_t fr2,
                              vertex_t to2) {
  struct move_t *move = malloc(sizeof(struct move_t));
  if (!move) {
    fprintf(stderr, "Erreur d'allocation mémoire pour le mur\n");
    exit(EXIT_FAILURE);
  }
  move->t       = WALL;
  move->c       = color;
  move->e[0].fr = fr1;
  move->e[0].to = to1;
  move->e[1].fr = fr2;
  move->e[1].to = to2;
  return move;
}

//

// function int  a void take an array and return this array full with all
// available moves that we could do and return the nuber of them
int availableMovess(struct move_t moves[], struct graph_t *graph, struct player_tt *p,
                    vertex_t opponent) {
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

int availableMoves(struct move_t moves[], struct graph_t *graph, struct player_tt *p,
                   vertex_t opponent) {
  int nb_moves = 0;
  nb_moves += availableMovess(moves, graph, p, opponent);
  //  nb_moves += availableWalls(moves + nb_moves, graph, p, opponent);
  return nb_moves;
}

struct move_t generate_random_valid_move(struct graph_t *g, struct player_tt *p,
                                         vertex_t opponent_pos) {
  int m = (int)((sqrt(4 * g->num_vertices + 1) + 1) / 3);
  int l, c;
  index_to_axial(p->position, m, &l, &c, g->type);

  int directions[6] = {1, 2, 3, 4, 5, 6};

  while (1) {
    // Mélanger les directions
    for (int i = 5; i > 0; --i) {
      int j         = rand() % (i + 1);
      int tmp       = directions[i];
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

      vertex_t dest = axial_to_index(l2, c2, m, g->type);

      if (valid_move(g, p, dest, opponent_pos)) {
        return (struct move_t){.t = MOVE, .c = p->c, .m = dest};
      }
    }
  }
}

/*
void  test_humping_over_opp(){
  //test valid_move
  struct graph_t *g = createGraph(6, TRIANGULAR);
  g->num_objectives = 1 ;
  g->objectives[0] = 29;
  struct player_tt p;
  p.position = 58;
  p.last_position = 67;
  p.c = 0;
  vertex_t opponent_pos = 48;
  vertex_t target = 29;
  int *path = malloc(g->num_vertices * sizeof(int));
  print_hex_grid(g);
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
  p.position = 58;
  p.last_position = 67;
  p.c = 0;

  int result2 = valid_move(g, &p, 38, 48);
  printf("Valid move from %u to %u: %d\n", p.position, 30, result2);
}

int main() {
  srand(time(NULL));
  test_humping_over_opp();
  return 0;
}
*/
