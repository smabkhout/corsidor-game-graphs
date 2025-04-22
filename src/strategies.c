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
    struct move_t previous_moves[2];// dernier coup pour chaque joueur
    vertex_t previous_positions[2] ; 
};

struct scored_move {
    int score;
    struct move_t move;
};

// Fonction pour appliquer un mouvement à l'état du jeu
    


struct game_state applyy_move(const struct game_state *state, struct move_t move) {
    struct game_state new_state = *state;
    new_state.previous_positions[move.c] = state->previous_moves[move.c].m;
    new_state.previous_moves[move.c] = move;
    return new_state;
}

#include <stdbool.h>
#include <limits.h>

int normalized_shortest_path(struct game_state *state, int color) {
    vertex_t current_pos = state->previous_moves[color].m;
    vertex_t *objectives = state->graph->objectives;
    unsigned int num_obj = state->graph->num_objectives;

    // Vérifier si déjà sur un objectif
    for (unsigned int i = 0; i < num_obj; i++) {
        if (current_pos == objectives[i]) {
            return 0; // Distance minimale
        }
    }

    // Implémentation BFS pour trouver le plus court chemin
    int *distances = malloc(state->graph->num_vertices * sizeof(int));
    bool *visited = malloc(state->graph->num_vertices * sizeof(bool));
    
    for (vertex_t v = 0; v < state->graph->num_vertices; v++) {
        distances[v] = INT_MAX;
        visited[v] = false;
    }

    vertex_t *queue = malloc(state->graph->num_vertices * sizeof(vertex_t));
    size_t front = 0, back = 0;

    distances[current_pos] = 0;
    visited[current_pos] = true;
    queue[back++] = current_pos;

    int shortest_path = INT_MAX;

    while (front < back) {
        vertex_t u = queue[front++];

        // Vérifier si c'est un objectif
        for (unsigned int i = 0; i < num_obj; i++) {
            if (u == objectives[i]) {
                shortest_path = distances[u];
                front = back; // Sortir de la boucle
                break;
            }
        }

        // Parcourir les voisins
        for (size_t k = 0; k < state->graph->t->nz; k++) {
            vertex_t row = state->graph->t->i[k];
            vertex_t col = state->graph->t->p[k];
            unsigned int val = state->graph->t->data[k];

            if (row == u && val != WALL_DIR && !visited[col]) {
                visited[col] = true;
                distances[col] = distances[u] + 1;
                queue[back++] = col;
            }
        }
    }

    free(distances);
    free(visited);
    free(queue);

    // Normalisation entre 0 et 100 (0 = sur objectif, 100 = inaccessible)
    if (shortest_path == INT_MAX) {
        return 100; // Aucun chemin trouvé
    }

    // Normalisation basée sur la taille du graphe
    int max_possible = state->graph->num_vertices / 2;
    return (shortest_path * 100) / max_possible;
}
int harmonic_potential(struct game_state *state, int color) {
    vertex_t current_pos = state->previous_moves[color].m;
    vertex_t opp_pos = state->previous_moves[1 - color].m;
    vertex_t *objectives = state->graph->objectives;
    unsigned int num_obj = state->graph->num_objectives;

    // Calcul de la distance moyenne aux objectifs
    int sum_dist = 0;
    int reachable_obj = 0;

    for (unsigned int i = 0; i < num_obj; i++) {
        int dist = normalized_shortest_path(state, color);
        if (dist < 100) { // Seulement si l'objectif est atteignable
            sum_dist += dist;
            reachable_obj++;
        }
    }

    if (reachable_obj == 0) return -50; // Aucun objectif atteignable

    int avg_dist = sum_dist / reachable_obj;

    // Calcul similaire pour l'adversaire
    struct game_state opp_state = *state;
    opp_state.previous_moves[color].m = opp_pos;
    int opp_avg_dist = harmonic_potential(&opp_state, 1 - color);

    // Potentiel harmonique: différence entre nos distances et celles de l'adversaire
    return opp_avg_dist - avg_dist;
}


int pawn_on_goal_side(struct game_state *state, int color) {
    vertex_t current_pos = state->previous_moves[color].m;
    vertex_t *objectives = state->graph->objectives;
    unsigned int num_obj = state->graph->num_objectives;
    int m = (int)(sqrt(4 * state->graph->num_vertices + 1) + 1) / 3;

    // Trouver la position moyenne des objectifs
    int avg_l = 0, avg_c = 0;
    int valid_obj = 0;

    for (unsigned int i = 0; i < num_obj; i++) {
        int l, c;
        index_to_axial(objectives[i], m, &l, &c);
        avg_l += l;
        avg_c += c;
        valid_obj++;
    }

    if (valid_obj == 0) return 0;

    avg_l /= valid_obj;
    avg_c /= valid_obj;

    // Position actuelle du joueur
    int l, c;
    index_to_axial(current_pos, m, &l, &c);

    // Vecteur vers le centre des objectifs
    int vec_l = avg_l - l;
    int vec_c = avg_c - c;

    // Score basé sur la direction
    if (color == 0) { // Joueur 0 (BLACK)
        if (vec_l > 0) return 1; // Se dirige vers les l positifs
        if (vec_l < 0) return -1; // S'éloigne
    } else { // Joueur 1 (WHITE)
        if (vec_l < 0) return 1; // Se dirige vers les l négatifs
        if (vec_l > 0) return -1; // S'éloigne
    }

    return 0;
}

int evaluate(struct game_state *state, int color){

    /*
    int f1 = normalized_shortest_path(state, color);
    int f2 = normalized_shortest_path(state, 1 - color);
    int f3 = harmonic_potential(state, color);
    int f4 = pawn_on_goal_side(state, color);

    // Coefficients ajustables
    return 15 * (100 - f1)    // Maximiser notre proximité aux objectifs
         - 12 * (100 - f2)     // Minimiser la proximité de l'adversaire
         + 8 * f3              // Potentiel harmonique
         + 10 * f4;            // Position stratégique
    */
   //si le joueurs arrive a l'objectif 1 si non 0 
   if (state->previous_moves[color].m == state->graph->objectives[0]) {
       return 10000;
   }
    return 0 ; 

}
/*
struct scored_move negamax(struct game_state *state, int depth, int alpha, int beta, int color) {
    if (depth == 0 || state->graph->num_edges == 0) {
        int score = color * evaluate(state, color);
        return (struct scored_move){ .score = score };
    }
    int x, y;
    if (color== 1) {
              x = 0 ;
              y = 1 ; 
    } else {
              x = 1 ;
              y = 0 ; 
    }
    struct player_tt *player1 = malloc(sizeof(struct player_tt));
    player1->position = state->previous_moves[x].m;
    player1->c = x ; 
    player1->last_position = state->previous_positions[x] ;
    struct move_t legal_moves[128];
    int num_moves = availableMoves(legal_moves, state->graph,player1, state->previous_moves[y].m);

    struct scored_move best = { .score = -1000000 };

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = applyy_move(state, legal_moves[i]);

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

struct scored_move negamax_naive(struct game_state *state, int depth, int color) {
    if (depth == 0 || state->graph->num_edges == 0) {
        int score = color * evaluate(state, color);
        return (struct scored_move){ .score = score };
    }

    int x = (color == 1) ? 0 : 1;
    int y = 1 - x;

    struct player_tt *player = malloc(sizeof(struct player_tt));
    player->position = state->previous_moves[x].m;
    player->c = x;
    player->last_position = state->previous_positions[x];

    struct move_t legal_moves[128];
    int num_moves = availableMoves(legal_moves, state->graph, player, state->previous_moves[y].m);

    free(player);

    struct scored_move best = { .score = -1000000 };

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = applyy_move(state, legal_moves[i]);
        struct scored_move result = negamax_naive(&next, depth - 1, -color);
        int score = -result.score;

        if (score > best.score) {
            best.score = score;
            best.move = legal_moves[i];
        }
    }

    return best;
}


struct move_t iterative_negamax(struct game_state *state, int time_limit_ms) {
    int depth = 5;
    struct scored_move best = { .score = -1000000 };

    clock_t start = clock();

    while ((clock() - start) * 1000 / CLOCKS_PER_SEC < time_limit_ms) {
        struct scored_move current = negamax(state, depth, -1000000, 1000000, 1);
        best = current;
        depth++;
    }

    return best.move;
}

struct move_t iterative_negamax_naive(struct game_state *state, int max_depth) {
    struct scored_move best = { .score = -1000000 };

    for (int depth = 1; depth <= max_depth; depth++) {
        struct scored_move current = negamax_naive(state, depth, 1);
        best = current;
    }

    return best.move;
}

*/




int is_game_over(struct game_state *state) {
    // Vérifie si un joueur a atteint l'objectif
    for (int i = 0; i < 2; i++) {
        if (state->previous_moves[i].m == state->graph->objectives[0]) {
            return 1; // Jeu terminé
        }
    }
    return 0; // Jeu non terminé
}


struct scored_move negamax_ab(struct game_state *state, int depth, int alpha, int beta, int color) {
    if (depth == 0 || is_game_over(state)) {
        int score = evaluate(state, color);
        return (struct scored_move){ .score = score };
    }

    int self = (color == 1) ? 0 : 1;
    int opponent = 1 - self;

    struct move_t legal_moves[128];
    struct player_tt player = {
        .position = state->previous_moves[self].m,
        .last_position = state->previous_positions[self],
        .c = self
    };
    int num_moves = availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

    struct scored_move best = { .score = -1000000 };

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = applyy_move(state, legal_moves[i]);
        struct scored_move result = negamax_ab(&next, depth - 1, -beta, -alpha, -color);
        int score = -result.score;

        if (score > best.score) {
            best.score = score;
            best.move = legal_moves[i];
        }

        if (score > alpha)
            alpha = score;

        if (alpha >= beta)
            break; // Coupure alpha-bêta
    }

    return best;
}



struct scored_move negamax_naive(struct game_state *state, int depth, int color) {
    if (depth == 0 || is_game_over(state)) {
        int score = evaluate(state, color); // Supposé relatif au joueur color
        return (struct scored_move){ .score = score };
    }

    int self = (color == 1) ? 0 : 1;
    int opponent = 1 - self;

    struct move_t legal_moves[128];
    struct player_tt player = {
        .position = state->previous_moves[self].m,
        .last_position = state->previous_positions[self],
        .c = self
    };
    int num_moves = availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

    struct scored_move best = { .score = -1000000 };

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = applyy_move(state, legal_moves[i]);
        struct scored_move result = negamax_naive(&next, depth - 1, -color);
        int score = -result.score; // NegaMax : inversion du signe

        if (score > best.score) {
            best.score = score;
            best.move = legal_moves[i];
        }
    }

    return best;
}


struct scored_move minMax(struct game_state *state, int depth, int maximizingPlayer) {
    int self = maximizingPlayer ? 0 : 1;
    int opponent = 1 - self;
    if (depth == 0 || is_game_over(state)) {
        int score = evaluate(state,self); // Score du point de vue du MAX
        return (struct scored_move){.move =state->previous_moves[self] , .score = score };
    }

    struct move_t legal_moves[128];
    struct player_tt player = {
        .position = state->previous_moves[self].m,
        .last_position = state->previous_positions[self],
        .c = self
    };
    int num_moves = availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

    struct scored_move best;
    best.score = maximizingPlayer ? -1000000 : 1000000;

    for (int i = 0; i < num_moves; i++) {
        
        struct game_state next = applyy_move(state, legal_moves[i]);
        //test print
        printf("move a traitee %d\n" ,legal_moves[i].m) ;
        struct scored_move result = minMax(&next, depth - 1, maximizingPlayer);
        printf("score %d \n" , result.score) ;
        if (maximizingPlayer) {
            if (result.score > best.score) {
                best.score = result.score;
                best.move = legal_moves[i];
            }
        } else {
            if (result.score < best.score) {
                best.score = result.score;
                best.move = legal_moves[i];
            }
        }
        //print old score and new if it's updated 
        
    }

    return best;
}


#define INF 1000000

struct scored_move minMaxi(struct game_state *state, int depth, int maximizingPlayer, int originalMaxPlayer) {
    if (depth == 0 || is_game_over(state)) {
        int score = evaluate(state, originalMaxPlayer); // Always evaluate from the original maximizer's view
        return (struct scored_move){ .move = state->previous_moves[originalMaxPlayer], .score = score };
    }

    int self = maximizingPlayer ? originalMaxPlayer : (1 - originalMaxPlayer);
    int opponent = 1 - self;

    struct move_t legal_moves[128];
    struct player_tt player = {
        .position = state->previous_moves[self].m,
        .last_position = state->previous_positions[self],
        .c = self
    };
    int num_moves = availableMoves(legal_moves, state->graph, &player, state->previous_moves[opponent].m);

    struct scored_move best;
    best.score = maximizingPlayer ? -INF : INF;

    for (int i = 0; i < num_moves; i++) {
        struct game_state next = applyy_move(state, legal_moves[i]);

        printf("Move being processed: %d\n", legal_moves[i].m);

        struct scored_move result = minMaxi(&next, depth - 1, !maximizingPlayer, originalMaxPlayer);

        printf("Resulting score: %d\n", result.score);

        if (maximizingPlayer) {
            if (result.score > best.score) {
                best.score = result.score;
                best.move = legal_moves[i];
            }
        } else {
            if (result.score < best.score) {
                best.score = result.score;
                best.move = legal_moves[i];
            }
        }
    }

    return best;
}



void test_evaluation_functions() {
    printf("=== Test des fonctions d'évaluation ===\n");
    
    // Créer un graphe de test (m=3)
    int m =5;
    struct graph_t* graph = createGraph(m, TRIANGULAR);
    
    // Configurer des objectifs (par exemple, les bords opposés)
    graph->num_objectives = 1;
    graph->objectives = malloc(3 * sizeof(vertex_t));
    graph->objectives[0] = axial_to_index(0, 0, m);  // Bord nord-ouest
    printf("objectif are in %d \n " , axial_to_index(0,0, m)); 
    //graph->objectives[1] = axial_to_index(-2, 1, m);  // Bord sud-est
    //graph->objectives[2] = axial_to_index(0, 2, m);   // Bord est

    // Initialiser l'état du jeu
    struct game_state state;
    state.graph = graph;
    
    // Position des joueurs
    state.previous_moves[0].m = axial_to_index(1, 1, m);  // Joueur 0 au centre
    state.previous_moves[0].c = 0;
    state.previous_moves[0].t = MOVE;
    state.previous_moves[1].c = 1;  
    state.previous_moves[1].t = MOVE;
    state.previous_moves[1].m = axial_to_index(1, -1, m); // Joueur 1 à côté
    state.previous_positions[0] = axial_to_index(0, 1, m);
    state.previous_positions[1] = axial_to_index(0, -1, m);
    /*
    // Tester les fonctions
    printf("Test normalized_shortest_path:\n");
    int dist_p0 = normalized_shortest_path(&state, 0);
    int dist_p1 = normalized_shortest_path(&state, 1);
    printf("Distance joueur 0: %d\n", dist_p0);
    printf("Distance joueur 1: %d\n", dist_p1);

    printf("\nTest harmonic_potential:\n");
    int pot_p0 = harmonic_potential(&state, 0);
    int pot_p1 = harmonic_potential(&state, 1);
    printf("Potentiel joueur 0: %d\n", pot_p0);
    printf("Potentiel joueur 1: %d\n", pot_p1);

    printf("\nTest pawn_on_goal_side:\n");
    int side_p0 = pawn_on_goal_side(&state, 0);
    int side_p1 = pawn_on_goal_side(&state, 1);
    printf("Position joueur 0: %d\n", side_p0);
    printf("Position joueur 1: %d\n", side_p1);
    */


    printf("\nTest evaluate:\n");
    int eval_p0 = evaluate(&state, 0);
    int eval_p1 = evaluate(&state, 1);
    printf("Evaluation joueur 0: %d\n", eval_p0);
    printf("Evaluation joueur 1: %d\n", eval_p1);

     // Test avec une profondeur limitée pour un résultat rapide
    printf("Calcul du meilleur coup pour le joueur 0...\n");
    //print available moves
    struct move_t legal_moves[1069];
    struct player_tt *player = malloc(sizeof(struct player_tt));
    player->position = state.previous_moves[0].m;
    player->c = 0;
    player->last_position = state.previous_positions[0];
    //player->walls = 10;
    //player->c = 0;
    //player->last_position = state.previous_positions[0];

    //print previous and current positions
    printf("Position actuelle du joueur 0: %d\n", state.previous_moves[0].m);
    printf("Position précédente du joueur 0: %d\n", state.previous_positions[0]);
    int num_moves = availableMoves(legal_moves, graph,player , state.previous_moves[1].m);
    printf("Nombre de déplacements possibles : %d\n", num_moves);
    for (int i = 0; i < num_moves; i++) {
        printf(" %u ,",  legal_moves[i].m);
    }
    printf("\n");


    struct scored_move result = negamax_ab(&state, 10, -1, 1, 1);
    struct scored_move result2 = negamax_naive(&state, 4, 1);
    struct scored_move best_move = minMax(&state, 1, 1);
    struct scored_move best_movee = minMaxi(&state, 1, 1, 1);
    
    printf("Meilleur coup trouvé:\n");
    printf("Type: %s\n", result.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", result.move.m);
    printf("Score: %d\n", result.score);
    

    printf("Meilleur coup trouvé (naif):\n");
    printf("Type: %s\n", result2.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", result2.move.m);
    printf("Score: %d\n", result2.score);
    

    printf("Meilleur coup trouvé (minMax):\n");
    printf("Type: %s\n", best_move.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", best_move.move.m);
    printf("Score: %d\n", best_move.score);

    printf("Meilleur coup trouvé (minMaxi):\n");
    printf("Type: %s\n", best_movee.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", best_movee.move.m);
    printf("Score: %d\n", best_movee.score);
    
    state.previous_moves[0].m = axial_to_index(3, 0, m);  // Joueur 0 au centre
    state.previous_moves[0].c = 0;
    state.previous_moves[0].t = MOVE;
    state.previous_moves[1].c = 1;  
    state.previous_moves[1].t = MOVE;
    state.previous_moves[1].m = axial_to_index(1, -1, m); // Joueur 1 à côté
    state.previous_positions[0] = axial_to_index(2, 0, m);
    state.previous_positions[1] = axial_to_index(0, -1, m);

    //sumiler le je et continue jusqu'a ce que le joueur 0 arrive a l'objectif avec chaque une des methodes 
    while (!is_game_over(&state)) {
        //print position actuelle du joueur 0   
        printf("Position actuelle du joueur 0: %d\n", state.previous_moves[0].m);
        // Appliquer le meilleur coup trouvé
        state = applyy_move(&state, result.move);
        printf("Coup appliqué: %d\n", result.move.m);
        // Mettre à jour l'état du jeu
        state.previous_moves[0] = result.move;
        state.previous_positions[0] = state.previous_moves[0].m;
        // Recalculer le meilleur coup
        result = minMax(&state, 4, 1);
    }

    if (result.move.t == MOVE) {
        int valid = valid_move(graph, &player, result.move.m, state.previous_moves[1].m) ;
        printf("Coup valide: %s\n", valid ? "OUI" : "NON");
        //assert(valid);
    }else {
       int valid = valid_wall(graph, &player, result.move);
        printf("Mur valide: %s\n", valid ? "OUI" : "NON");
       // assert(valid);
    }
    
    printf("Test réussi!\n");
    //graph_free(graph);

    // Nettoyage
    //free(graph->objectives);
    //graph_free(graph);
    
    printf("=== Tests terminés ===\n");
}



int main() {
    test_evaluation_functions();
    //test_negamax();
    
    return 0;
}


