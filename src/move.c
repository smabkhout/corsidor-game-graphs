#include "move_functions.h"
#include <stdio.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <gsl/gsl_spblas.h>
#include <math.h>







struct move_t create_move(enum player_color_t color, enum move_type_t type, vertex_t vertex, struct edge_t edges[2]) {
    struct move_t move;
    move.c = color;
    move.t = type;
    move.m = vertex;
    if (type == WALL) {
        move.e[0] = edges[0];
        move.e[1] = edges[1];
    }
    return move;
}


int is_empty_vertice( vertex_t n , vertex_t pos_other_player){
        if (pos_other_player == n){
            return 0;
        }
    return 1;
}

/*int is_empty_vertice( vertex_t n , struct board_t *board){
        for(int i=0 ; i<NUM_PLAYERS ; i++){
            if (board->graph->start[i] == n ) return 0 ; 
        }
    return 1;
}
*/


int is_valid_move(const struct move_t* move, const struct graph_t* graph) {
    if (!move || !graph) {
        return 0;  
    }
    if (move->t == MOVE) {
        if (move->m >= graph->num_vertices) {
            return 0;  
        }
    } else if (move->t == WALL) {
        if (move->e[0].fr >= graph->num_vertices || move->e[0].to >= graph->num_vertices ||
            move->e[1].fr >= graph->num_vertices || move->e[1].to >= graph->num_vertices) {
            return 0;  
        }
    }
    return 1;  
}

int can_move(struct graph_t *graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player) {
    if (!(is_empty_vertice(b, pos_other_player)) || !(gsl_spmatrix_uint_get(graph->t, pos_player, b) > 0)) {
        return 0;
    }
    struct move_t move=create_move(NO_COLOR, MOVE, b, NULL);
    if (!is_valid_move(&move, graph)){
        return 0;
    }
    return 1;
}

int is_connected(struct graph_t *graph, vertex_t v1, vertex_t v2){
    if(!graph){
        return 0;
    }
    if (gsl_spmatrix_uint_get(graph->t, v1, v2)>0){
        return 1;
    }
    return 0;

}

int can_place_wall(struct graph_t *graph, struct edge_t e[2]) {
    if(e[0].fr!=e[1].fr && e[0].to!=e[1].fr && e[0].fr!=e[1].to && e[0].to!=e[1].to ){
        return 0;
    }
    if(!is_connected(graph, e[0].fr, e[0].to)||!is_connected(graph, e[1].fr, e[1].to) ){
        return 0;
    }
    struct move_t move = create_move(NO_COLOR, WALL, 0, e);
    if(!is_valid_move(&move, graph)){
        return 0;
    }
    return 1;
}







int distance_minimal(struct graph_t * graph, int d[], int visited[], unsigned int n){
    int min=INT_MAX;
    unsigned int index = graph->num_vertices;
    for(unsigned int i=0;i<n;i++){
        if(!visited[i] && d[i]<min){
            min = d[i];
            index = i;
        }

    }
    return index;
}


void dijistra ( struct graph_t * graph, vertex_t a, vertex_t b, int d[graph->num_vertices], int prev[graph->num_vertices]){
    int visited[graph->num_vertices];
    for (unsigned int i=0; i<graph->num_vertices; i++){
        d[i]=INT_MAX;
        prev[i]=-1;
        visited[i]=0;
    }
    d[a]=0;
    while(1){
        unsigned int index_min =  distance_minimal(graph, d, visited, graph->num_vertices);
        if (index_min >= graph->num_vertices){
            break;
        }
        visited[index_min] = 1; 
        for (unsigned int j=0;j<graph->num_vertices; j++){
            int poids=gsl_spmatrix_uint_get(graph->t, index_min, j);
            if(poids >0 && d[index_min]+ poids < d[j] && !visited[j]){
                d[j]=d[index_min]+poids;
                prev[j]=index_min;
            }

        }
        if(index_min == b){
            break;
        }

    }
    
}


/*

void test_apply_move() {
    struct graph_t graph;
    graph.num_vertices = 4;
    graph.start[BLACK] = 0;  // Player BLACK starts at vertex 0
    graph.start[WHITE] = 3;  // Player WHITE starts at vertex 3

    // Create and apply a valid player move (BLACK moves to vertex 2)
    struct move_t move1 = create_move(BLACK, MOVE, 2, NULL);

    printf("Player BLACK new position: %u\n", graph.start[BLACK]);

    // Create and apply a wall move (WHITE blocks the edge between vertices 1 and 2)
    struct edge_t edges[2] = {{1, 2}, {2, 1}};
    struct move_t move2 = create_move(WHITE, WALL, 0, edges);
    printf("\nApplying Move 2 (Player WHITE blocks edge between 1 and 2):\n");

    // Check the adjacency matrix after applying the wall move
    printf("Adjacency matrix after wall move:\n");
    for (unsigned int i = 0; i < graph.num_vertices; i++) {
        for (unsigned int j = 0; j < graph.num_vertices; j++) {
            printf("%u ", gsl_spmatrix_uint_get(graph.t, i, j));
        }
        printf("\n");
    }
}*/




/*int main() {
    printf("=== Test create_move ===\n");
    test_create_move();

    printf("\n=== Test is_valid_move ===\n");
    test_is_valid_move();

    printf("\n=== Test apply_move ===\n");
    test_apply_move();

    return 0;
}
*/






void availableMoves(struct move_t* moves[], struct graph_t *graph, int id_ofplayer, struct move_t* previous_move) {
    int nb_moves = 0;

    vertex_t current = get_player_position(id_ofplayer);
    vertex_t opponent = get_player_position( (id_ofplayer+1)%2);
    enum dir_t prev_dir = get_direction_from_move(previous_move);

    // === Déplacements ===

    // Parcours des voisins immédiats
    for (vertex_t i = 0 ; i<graph->num_vertices ; i++){
        if (i != opponent && is_connected(graph , current , i )) {
            moves[nb_moves++] = make_move_move(id_ofplayer, i);
        }

        // Vérifie si saut par-dessus est possible
        if (i == opponent) {
            for (vertex_t j = 0 ; j<graph->num_vertices ;j++)/*each neighbor_of_opponent w in graph[opponent] */{
                if (j != current && j != opponent &&is_connected(graph , opponent , j )) {
                    moves[nb_moves++] = make_move_move(id_ofplayer, j);
                }
            }
        }
    }
    /*
    // === Déplacements longue distance (directionnelle) ===
    if (prev_dir is known) {
        for d in allowed_directions_based_on(prev_dir) {
            for k = 1 to max_range(d, prev_dir) {
                vertex_t target = move_in_direction(graph, current, d, k);
                if (target != INVALID && is_path_clear(current, target, graph) && is_free(target)) {
                    moves[nb_moves++] = make_move_move(id_ofplayer, target);
                }
            }
        }
    }

    // === Placement de murs ===
    if (get_remaining_walls(id_ofplayer) > 0) {
        for each vertex fr in graph {
            for each direction d1, d2 consécutifs autour de fr {
                edge_t e1 = {fr, get_neighbor(fr, d1)};
                edge_t e2 = {fr, get_neighbor(fr, d2)};
                if (edges_are_valid(e1, e2, graph) && !edges_already_blocked(e1, e2)) {
                    // Simuler le blocage temporairement
                    block_edges(graph, e1, e2);
                    if (both_players_have_path_to_goal()) {
                        moves[nb_moves++] = make_move_wall(id_ofplayer, e1, e2);
                    }
                    unblock_edges(graph, e1, e2);
                }
            }
        }
    }
    */
    // Terminer le tableau avec NULL si besoin
    moves[nb_moves] = NULL;
}