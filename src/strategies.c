#include <stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "player.h"
#include "move2.h"
#include "move.h"
#include "graph_functions.h"
#include <string.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <gsl/gsl_spblas.h>
#include <math.h>
#include <time.h>

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