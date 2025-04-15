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
    
struct game_state applyy_move(struct game_state *state, struct move_t legal_move) {
    struct game_state new_state = *state;
    new_state.previous_moves[legal_move.c] = legal_move;
    new_state.previous_positions[legal_move.c] = legal_move.m;
    return new_state;
}


#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

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

int evaluate(struct game_state *state, int color) {
    int f1 = normalized_shortest_path(state, color);
    int f2 = normalized_shortest_path(state, 1 - color);
    int f3 = harmonic_potential(state, color);
    int f4 = pawn_on_goal_side(state, color);

    // Coefficients ajustables
    return 15 * (100 - f1)    // Maximiser notre proximité aux objectifs
         - 12 * (100 - f2)     // Minimiser la proximité de l'adversaire
         + 8 * f3              // Potentiel harmonique
         + 10 * f4;            // Position stratégique
}
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


void test_evaluation_functions() {
    printf("=== Test des fonctions d'évaluation ===\n");
    
    // Créer un graphe de test (m=3)
    int m = 3;
    struct graph_t* graph = createGraph(m, TRIANGULAR);
    
    // Configurer des objectifs (par exemple, les bords opposés)
    graph->num_objectives = 3;
    graph->objectives = malloc(3 * sizeof(vertex_t));
    graph->objectives[0] = axial_to_index(2, -1, m);  // Bord nord-ouest
    graph->objectives[1] = axial_to_index(-2, 1, m);  // Bord sud-est
    graph->objectives[2] = axial_to_index(0, 2, m);   // Bord est

    // Initialiser l'état du jeu
    struct game_state state;
    state.graph = graph;
    
    // Position des joueurs
    state.previous_moves[0].m = axial_to_index(0, 0, m);  // Joueur 0 au centre
    state.previous_moves[1].m = axial_to_index(1, -1, m); // Joueur 1 à côté
    state.previous_positions[0] = axial_to_index(0, 0, m);
    state.previous_positions[1] = axial_to_index(1, -1, m);

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

    printf("\nTest evaluate:\n");
    int eval_p0 = evaluate(&state, 0);
    int eval_p1 = evaluate(&state, 1);
    printf("Evaluation joueur 0: %d\n", eval_p0);
    printf("Evaluation joueur 1: %d\n", eval_p1);

     // Test avec une profondeur limitée pour un résultat rapide
    printf("Calcul du meilleur coup pour le joueur 0...\n");
    struct scored_move result = negamax(&state, 2, -1000000, 1000000, 0);
    
    printf("Meilleur coup trouvé:\n");
    printf("Type: %s\n", result.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", result.move.m);
    printf("Score: %d\n", result.score);
    
    // Vérifier que le coup est valide
    struct player_tt player;
    player.position = state.previous_moves[0].m;
    player.last_position = state.previous_positions[0];
    player.c = 0;
    
    if (result.move.t == MOVE) {
        int valid = valid_move(graph, &player, result.move.m, state.previous_moves[1].m);
        printf("Coup valide: %s\n", valid ? "OUI" : "NON");
        assert(valid);
    }// else {
       // int valid = valid_wall(graph, &player, result.move);
       // printf("Mur valide: %s\n", valid ? "OUI" : "NON");
       // assert(valid);
    //}
    
    printf("Test réussi!\n");
    //graph_free(graph);

    // Nettoyage
    //free(graph->objectives);
    //graph_free(graph);
    
    printf("=== Tests terminés ===\n");
}



void test_negamax() {
    printf("=== Test de l'algorithme Negamax ===\n");
    
    // Créer un petit graphe (m=3 pour un test rapide)
    int m = 3;
    struct graph_t* graph = createGraph(m, TRIANGULAR);
    
    // Initialiser l'état du jeu
    struct game_state state;
    state.graph = graph;
    
    // Positions initiales des joueurs
    state.previous_moves[0].m = axial_to_index(0, 0, m);  // Joueur 0 au centre
    state.previous_moves[1].m = axial_to_index(1, -1, m); // Joueur 1 à côté
    state.previous_positions[0] = axial_to_index(0, 1, m);
    state.previous_positions[1] = axial_to_index(1, 0, m);
    
    // Test avec une profondeur limitée pour un résultat rapide
    printf("Calcul du meilleur coup pour le joueur 0...\n");
    struct scored_move result = negamax(&state, 2, -1000000, 1000000, 0);
    
    printf("Meilleur coup trouvé:\n");
    printf("Type: %s\n", result.move.t == MOVE ? "MOVE" : "WALL");
    printf("Destination: %d\n", result.move.m);
    printf("Score: %d\n", result.score);
    
    // Vérifier que le coup est valide
    struct player_tt player;
    player.position = state.previous_moves[0].m;
    player.last_position = state.previous_positions[0];
    player.c = 0;
    
    if (result.move.t == MOVE) {
        int valid = valid_move(graph, &player, result.move.m, state.previous_moves[1].m);
        printf("Coup valide: %s\n", valid ? "OUI" : "NON");
        assert(valid);
    }// else {
       // int valid = valid_wall(graph, &player, result.move);
       // printf("Mur valide: %s\n", valid ? "OUI" : "NON");
       // assert(valid);
    //}
    
    printf("Test réussi!\n");
    //graph_free(graph);
}

int main() {
    test_evaluation_functions();
    //test_negamax();
    
    return 0;
}