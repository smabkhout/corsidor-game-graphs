#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#include "move2.h"
#define NO_VERTEX ((vertex_t)(-1))

//enum graph_type_t type;
static struct board_t *board = NULL ; 
static unsigned int player_id;
//static vertex_t previous_position;
//static int has_played = 0;

char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[1];
}



void initialize(unsigned int id, struct graph_t* graph) {
    board = board_init();
    // board->graph = malloc(sizeof(struct graph_t));
    board->graph = graph;
      if (!board->graph) {
          fprintf(stderr, "Erreur allocation du graph\n");
          exit(EXIT_FAILURE);
      }
  
      // copy_graph(board->graph, graph); 
  
    printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , board->graph-> num_vertices , board->graph->num_edges , board->graph->num_objectives);
  
  }

int get_neighbors(struct graph_t* graph, vertex_t v, vertex_t* out, int max_out) {
    int count = 0;
    for (vertex_t i = 0; i < graph->num_vertices && count < max_out; i++) {
        if (gsl_spmatrix_uint_get(graph->t, v, i) != 0) {
            out[count++] = i;
        }
    }
    return count;
    }
struct move_t make_move_no_type() {
    struct move_t move;
    move.t = NO_TYPE;
    move.c = NO_COLOR;
    move.m = 0;
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;
    return move;
}
#include <limits.h>
#include <stdbool.h>

// Structure pour stocker les distances dans Dijkstra
struct distance_node {
  vertex_t vertex;
  int distance;
  bool visited;
};

// Fonction utilitaire pour trouver le sommet non visité avec la distance
// minimale
vertex_t min_distance_vertex(struct distance_node *nodes, size_t num_vertices) {
  int min_dist = INT_MAX;
  vertex_t min_vertex = 0;

  for (vertex_t v = 0; v < num_vertices; ++v) {
    if (!nodes[v].visited && nodes[v].distance <= min_dist) {
      min_dist = nodes[v].distance;
      min_vertex = v;
    }
  }

  return min_vertex;
}


int shortest_path_length(struct graph_t *g, vertex_t start, vertex_t objective,
                         vertex_t opponent_pos, vertex_t *path) {
  if (start == objective)
    return 0;
  int path_length = 0;

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

  unsigned int n = 3 * (m * m) - 3 * m + 1;
  // Initialisation des nœuds
  struct distance_node *nodes = malloc(n * sizeof(struct distance_node));
  if (!nodes)
    return -1;

  // Nouveau tableau pour reconstruire le chemin
  vertex_t *prev = malloc(n * sizeof(vertex_t));
  if (!prev) {
    free(nodes);
    return -1;
  }
  for (unsigned int i = 0; i < n; ++i) {
    prev[i] = -1;
  }

  for (vertex_t v = 0; v < n; ++v) {
    int l;
    int c;
    index_to_axial(v, m, &l, &c);
    if (!in_hexagon(l, c, m, 0, 0)) {
      nodes[v].vertex = -1;
      continue;
    }
    nodes[v].vertex = v;
    nodes[v].distance = INT_MAX;
    nodes[v].visited = false;
  }
  nodes[start].distance = 0;
  path[path_length] = start;

  // Algorithme de Dijkstra
  for (size_t i = 0; i < n; ++i) {
    vertex_t u = min_distance_vertex(nodes, n);
    if (u == objective || nodes[u].distance == INT_MAX) {
      break;
    }

    nodes[u].visited = true;

    // Parcourir tous les voisins de u
    for (vertex_t v = 0; v < n-1; ++v) {
      if (nodes[v].vertex == (unsigned int)-1) {
        printf("Le sommet %d n'est pas valide\n", v);
        continue;
      }
      // unsigned int edge_type = gsl_spmatrix_uint_get(g->t, u, v);
      struct player_tt p;
      p.position = u;
      p.c = 0;
      p.last_position = start;
      // Vérifier si l'arête existe et n'est pas un mur
      if (valid_move(g, &p, v, opponent_pos)) { // 0 = pas d'arête, 7 = mur
        if (!nodes[v].visited && nodes[u].distance + 1 < nodes[v].distance) {
          nodes[v].distance = nodes[u].distance + 1;
          prev[v] = u;
        }
      }
    }
  }

  int result = nodes[objective].distance;

  if (result != INT_MAX) {
    vertex_t current = objective;
    path_length = 0;

    // Reconstruction du chemin à l'envers
    while (current != (unsigned int)-1) {
      path[path_length++] = current;
      current = prev[current];
    }

    // Inverser le chemin
    for (int i = 0; i < path_length / 2; ++i) {
      vertex_t tmp = path[i];
      path[i] = path[path_length - 1 - i];
      path[path_length - 1 - i] = tmp;
    }

    path[path_length] = -1; // terminaison
  } else {
    path[0] = -1; // pas de chemin trouvé
    path_length = 0;
  }

  free(nodes);
  free(prev);

  return (result == INT_MAX) ? -1 : result;
}


struct move_t play(const struct move_t previous_move) {
    vertex_t my_pos = board->graph->start[player_id];
    vertex_t opp_pos = board->graph->start[(player_id + 1) % NUM_PLAYERS];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }
    // si board ->size_moves <4 ; fait un move aleatoire 
    if (board->size_moves < 4) {
        while(1){
        struct move_t move;
        move.t = MOVE;
        move.c = player_id;
        move.m = rand() % board->graph->num_vertices;
        if (valid_move(board->graph, player_id, move.m, my_pos)) {
            return move;
        }
        
        }
    }else {
    struct move_t *moove = malloc(sizeof(struct move_t));
    struct move_t availbel[128];
    struct player_tt p;
    p.position = my_pos;
    p.last_position = board->moves[board->size_moves - 4].m;
    p.c = player_id;

    int count = availableMoves(availbel, board->graph, &p, opp_pos);
    int score=0 ;
    for (int i = 0; i < count; i++) {
        struct move_t move;
        move = availbel[i];
        vertex_t *path = malloc(board->graph->num_vertices * sizeof(vertex_t));
        if (shortest_path_length(board->graph, move.m ,board->graph->objectives[0],opp_pos , path) > score) {
            score = shortest_path_length(board->graph, move.m ,board->graph->objectives[0],opp_pos , path);
            *moove = move;
             ;
            }
        free(path);
        }
        return *moove;
    }

    }
    
    






/*struct move_t play(const struct move_t previous_move) {
    struct move_t move;

    move.t = NO_TYPE;
    move.c = previous_move.c == NO_COLOR ? BLACK : (previous_move.c + 1) % 2;
    move.m = 0; 
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;

    printf("👻 Player %d plays a NO_TYPE move (mock behavior)\n", move.c);

    if (board != NULL) {
        add_move_to_board(board, move);
    } else {
        printf("Board non initialisé dans play()\n");
    }

    return move;
}*/


void finalize() {
    if (board) {
        board_free(board);
        board = NULL;
    }
}


