#include "move_functions.h"
#include <stdio.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <gsl/gsl_spblas.h>
#include <math.h>

#define NUM_DIRECTIONS 6





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
    if(e[0].fr!=e[1].fr){
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

enum dir_t get_direction(vertex_t from, vertex_t to, struct graph_t* graph) {
    return gsl_spmatrix_uint_get(graph->t, from, to);
}

struct move_t make_move_move(enum player_color_t color, vertex_t dest) {
    struct move_t move;
    move.t = MOVE;
    move.c = color;
    move.m = dest;
    return move;
}





int distance_minimal(int d[], int visited[], unsigned int n){
    int min=INT_MAX;
    unsigned int index = -1;
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
        unsigned int index_min =  distance_minimal(d, visited, graph->num_vertices);
        if (index_min == (unsigned int)-1){
            break;
        }
        visited[index_min] = 1; 
        for (unsigned int j=0;j<graph->num_vertices; j++){
            int dir=gsl_spmatrix_uint_get(graph->t, index_min, j);
            int poids =1;
            if(dir>0 && dir!= 7 && d[index_min]+poids< d[j] && !visited[j]){
                d[j]=d[index_min]+poids;
                prev[j]=index_min;
            }

        }
        if(index_min == b){
            break;
        }

    }
    
}


vertex_t find_closest_objective(struct graph_t* graph, vertex_t player_pos){
    vertex_t* objectives=graph->objectives;
    unsigned int num_obj=graph->num_objectives;
    int min_distance = INT_MAX;
    vertex_t closest_objective = player_pos;
    int distances[graph->num_vertices];
    int prev[graph->num_vertices];
    for (unsigned int i = 0; i < num_obj; i++) {
        vertex_t objective = objectives[i];
         dijistra(graph, player_pos, objective, distances, prev);
         int obj_distance=distances[objective];

         if (obj_distance < min_distance) {
            min_distance = obj_distance;
            closest_objective = objective;

          }
    }
    return closest_objective;
}


int is_path_clear(struct graph_t* graph, vertex_t from, enum dir_t dir, int dist, vertex_t opponent_pos, vertex_t* result) {
    vertex_t current = from;
    for (int i = 0; i < dist; i++) {
        int found = 0;
        for (vertex_t v = 0; v < graph->num_vertices; v++) {
            if (gsl_spmatrix_uint_get(graph->t, current, v) == dir) {
                if (v == opponent_pos) return 0; 
                current = v;
                found = 1;
                break;
            }
        }
        if (!found) return 0; 
    }
    *result = current;
    return 1;
}




static const enum dir_t directions[] = {NW, NE, E, SE, SW, W};
void get_side_dir_30(enum dir_t dir, enum dir_t* d1, enum dir_t* d2) {
    int index = -1;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        if (directions[i] == dir) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        *d1 = *d2 = NO_EDGE;
        return;
    }
    *d1 = directions[(index + NUM_DIRECTIONS - 1) % NUM_DIRECTIONS];
    *d2 = directions[(index + 1) % NUM_DIRECTIONS];
}


struct move_t find_best_move(struct graph_t* graph, vertex_t pos, vertex_t opponent, enum dir_t prev_dir, enum player_color_t color) {
    vertex_t dest;
    if (prev_dir != NO_EDGE) {
        for (int d = 3; d >= 1; d--) {
            if (is_path_clear(graph, pos, prev_dir, d, opponent, &dest)) {
                return make_move_move(color, dest);
            }
        }
    }
    if (prev_dir != NO_EDGE) {
        enum dir_t d1, d2;
        get_side_dir_30(prev_dir, &d1, &d2);
        enum dir_t side_dirs[2] = {d1, d2};
        for (int i = 0; i < 2; i++) {
            if (is_path_clear(graph, pos, side_dirs[i], 2, opponent, &dest)) {
                return make_move_move(color, dest);
            }
        }
    }

    for (vertex_t v = 0; v < graph->num_vertices; v++) {
        if (gsl_spmatrix_uint_get(graph->t, pos, v) > 0 && v != opponent && gsl_spmatrix_uint_get(graph->t, pos, v) != 7) {
            return make_move_move(color, v);
        }
    }

    struct move_t invalid = { .t = NO_TYPE, .c = color };
    return invalid;
}


















/*void availableMoves(struct move_t* moves[], struct graph_t *graph, int id_ofplayer, struct move_t* previous_move) {
    int nb_moves = 0;

    vertex_t current = get_player_position(id_ofplayer);
    vertex_t opponent = get_player_position( (id_ofplayer+1)%2);
    enum dir_t prev_dir = get_direction_from_move(previous_move);

    for (vertex_t i = 0 ; i<graph->num_vertices ; i++){
        if (i != opponent && is_connected(graph , current , i )) {
            moves[nb_moves++] = make_move_move(id_ofplayer, i);
        }
        if (i == opponent) {
            for (vertex_t j = 0 ; j<graph->num_vertices ;j++){ //each neighbor_of_opponent w in graph[opponent] 
                if (j != current && j != opponent &&is_connected(graph , opponent , j )) {
                    moves[nb_moves++] = make_move_move(id_ofplayer, j);
                }
            }
        }
    }
    moves[nb_moves] = NULL;
}*/