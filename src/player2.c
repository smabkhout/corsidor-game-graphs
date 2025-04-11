
/*#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>


#define INF 1000000000

//enum graph_type_t type;
static struct graph_t *graph2= NULL ; 





char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"Dina", "Salah"};
  return names[rand() % 2];
}


void copy_graph(struct graph_t* dest, const struct graph_t* src) {
    // Copier les champs simples
    dest->type = src->type;
    dest->num_vertices = src->num_vertices;
    dest->num_edges = src->num_edges;
    dest->num_objectives = src->num_objectives;
    
    memcpy(dest->start, src->start, sizeof(vertex_t) * NUM_PLAYERS); // Copier start[]

    // Copier la matrice creuse (sparse matrix)
    dest->t = gsl_spmatrix_uint_alloc(src->num_vertices, src->num_vertices);
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
    memcpy(dest->objectives, src->objectives, src->num_objectives * sizeof(vertex_t));  // Copie du tableau
}


void initialize(unsigned int id, struct graph_t* graph) {
  graph2= malloc(sizeof(struct graph_t)) ; 
  if (!graph2){
    puts("erreur dans l'allocation du graph pour player2 ") ; 
  }

  copy_graph(graph2 , graph) ; 

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , graph2-> num_vertices , graph2->num_edges , graph2->num_objectives);

}




//le code suivant il s'agit d'un code de minimax avec alpha beta pruning une strategie de jeu avancée pour l'instant est commenté , apres regler moves.c et le server on pourra l'utiliser

struct game_state {
    struct graph_t *graph;
    struct move_t previous_moves[2]; // last move for each player

} ; 

struct scored_move {
    int score;
    struct move_t move;
} ; 

struct game_state apply_move(struct game_state* state ,struct move_t legale_move ){
    (void)state;
    (void)legale_move;
    struct game_state next = *state;
    return next;
}


int availableMoves(struct move_t moves[], struct game_state *state, int color);
int normalized_shortest_path(struct game_state *state, int color);
int harmonic_potential(struct game_state *state, int color);
int pawn_on_goal_side(struct game_state *state, int color);


int evaluate(struct game_state *state, int color) {
    int f1 = normalized_shortest_path(state, color);
    int f2 = normalized_shortest_path(state, 1 - color);
    int f3 = harmonic_potential(state, color);
    int f4 = harmonic_potential(state, 1 - color);
    int f5 = pawn_on_goal_side(state, color);

    return 10*f1 - 8*f2 + 5*f3 - 5*f4 + 12*f5;
}

struct scored_move negamax(struct game_state *state, int depth, int alpha, int beta, int color) {
    if (depth == 0 || state->graph->num_edges == 0) {
        int score = color * evaluate(state, color);
        return (struct scored_move){ .score = score };
    }

    struct move_t legal_moves[128];
    int num_moves = availableMoves(legal_moves, state, color);

    struct scored_move best = { .score = -1000000 };

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = apply_move(state, legal_moves[i]);

        struct scored_move result = negamax(&next, depth - 1, -beta, -alpha, -color);
        int score = -result.score;

        if (score > best.score) {
            best.score = score;
            best.move = legal_moves[i];
        }

        if (score > alpha)
            alpha = score;
        if (alpha >= beta)
            break;
    }

    return best;
}

struct move_t iterative_negamax(struct game_state *state, int time_limit_ms) {
    int depth = 1;
    struct scored_move best = { .score = -1000000 };

    clock_t start = clock();

    while ((clock() - start) * 1000 / CLOCKS_PER_SEC < time_limit_ms) {
        struct scored_move current = negamax(state, depth, -1000000, 1000000, 1);
        best = current;
        depth++;
    }

    return best.move;
}


int is_connected1(struct graph_t *graph, vertex_t v1, vertex_t v2) {
    if (!graph || v1 >= graph->num_vertices || v2 >= graph->num_vertices) {
        return 0;  // Vérification de validité des indices
    }

    // Vérification de l'existence d'une arête entre v1 et v2
    if (gsl_spmatrix_uint_get(graph->t, v1, v2) > 0) {
        return 1;
    }

    return 0;
}


struct move_t play(const struct move_t previous_move) {
    struct move_t move;
    move.c = (previous_move.c+1)%2;
    move.t = MOVE;

    struct move_t moves[graph2->num_vertices];
    int numberOfMoves = availableMoves(moves, graph2, move.c, &previous_move);

    // Trouver une position voisine valide
    for ( unsigned int  i = 0; i < numberOfMoves; i++) {
        if (valid_move(graph2, moves[i].c, moves[i].m)) {
            //move.m = i;
            graph2->start[move.c] = moves[i].m; ; 
            printf("Player %d moves to vertex %u\n", move.c, move.m);
            return moves[i];
        }
    }

    // Si aucun mouvement valide, on reste sur place
    move.m = graph2->start[move.c]  ;
    return move;
}
*/

#include "graph.h"
#include "player.h"
#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include <string.h>
#include <gsl/gsl_spmatrix.h>

//enum graph_type_t type;
static struct graph_t *graph2= NULL ; 
//static unsigned int player_id;
//static vertex_t previous_position;
//static int has_played = 0;


char const* get_player_name()
{
  srand(time(NULL));
  char *names[] = {"adam", "rafiq"};
  return names[rand() % 2];
}


void copy_graph(struct graph_t* dest, const struct graph_t* src) {
    // Copier les champs simples
    dest->type = src->type;
    dest->num_vertices = src->num_vertices;
    dest->num_edges = src->num_edges;
    dest->num_objectives = src->num_objectives;
    
    memcpy(dest->start, src->start, sizeof(vertex_t) * NUM_PLAYERS); // Copier start[]

    // Copier la matrice creuse (sparse matrix)
    dest->t = gsl_spmatrix_uint_alloc(src->num_vertices, src->num_vertices);
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
    memcpy(dest->objectives, src->objectives, src->num_objectives * sizeof(vertex_t));  // Copie du tableau
}


void initialize(unsigned int id, struct graph_t* graph) {
  graph2= malloc(sizeof(struct graph_t)) ; 
  if (!graph2){
    fprintf(stderr, "Erreur d'allocation\n");
    exit(EXIT_FAILURE);
  }

  copy_graph(graph2 , graph) ; 

  printf("Player %d initialized on graph with %u vertices and %u edges , and with %u objectives\n", id , graph2-> num_vertices , graph2->num_edges , graph2->num_objectives);

}


/*struct move_t play(const struct move_t previous_move) {
    vertex_t my_pos = graph2->start[player_id];
    vertex_t opp_pos = graph2->start[(player_id + 1) % 2];

    if (previous_move.t == MOVE && previous_move.c != player_id) {
        opp_pos = previous_move.m;
    }

    enum dir_t prev_dir = NO_EDGE;
    if (has_played) {
        prev_dir = get_direction(previous_position, my_pos, graph2);
    } else {
        prev_dir = 3;
    }

    struct move_t move = find_best_move(graph2, my_pos, opp_pos, prev_dir, player_id);

    if (gsl_spmatrix_uint_get(graph2->t, my_pos, opp_pos) > 0) {
        for (vertex_t jump = 0; jump < graph2->num_vertices; jump++) {
            if (gsl_spmatrix_uint_get(graph2->t, opp_pos, jump) > 0 &&
                jump != my_pos && jump != opp_pos) {
                move = make_move_move(player_id, jump);
                break;
            }
        }
    }
    if (move.t == MOVE) {
        previous_position = my_pos;         
        graph2->start[player_id] = move.m;  
        has_played = 1;                     
    }
    return move;
}

void finalize() {
    if (graph1) {  
        gsl_spmatrix_uint_free(graph1->t);  
        free(graph1->objectives);  
        free(graph1);  
        graph1 = NULL;  
    }
}*/

struct move_t play(const struct move_t previous_move) {
    struct move_t move;

    move.t = NO_TYPE;
    move.c = previous_move.c == NO_COLOR ? BLACK : (previous_move.c + 1) % 2;
    move.m = 0; 
    move.e[0].fr = move.e[0].to = 0;
    move.e[1].fr = move.e[1].to = 0;

    printf("👻 Player %d plays a NO_TYPE move (mock behavior)\n", move.c);

    return move;
}


void finalize() {
    if (graph2) {  // Vérifier si le graphe existe avant de libérer
        gsl_spmatrix_uint_free(graph2->t);  // Libérer la matrice
        free(graph2->objectives);  // Libérer le tableau d'objectifs
        free(graph2);  // Libérer la structure du graphe
        graph2 = NULL;  // Éviter tout accès à une mémoire libérée
    }
}

