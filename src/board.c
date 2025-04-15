#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "graph_functions.h"
#define NUM_DIRECTIONS 6

struct board_t* board_init(){
    struct board_t *board=malloc(sizeof(struct board_t));
    board->moves=malloc(sizeof(struct move_t));
    board->size_moves=0;
    board->wall_count=0;
    board->current_positions[0]=2;
    board->current_positions[1]=5;
    return board;
}


/*void display_board(struct board_t *board) {
    for (int i = 0; i < board->size; i++) {
        for (int j = 0; j < board->size; j++) {
            char symbol;
            switch (board->cells[i][j]) {
                case BLACK: symbol = 'B'; break;
                case WHITE: symbol = 'W'; break;
                default: symbol = '.'; break; 
            }
            printf("%c ", symbol);
        }
        printf("\n");
    }
}*/


void add_move_to_board(struct board_t* board, struct move_t move){
    if (move.t == WALL ){
        board->wall_count++;

    }
    board->moves=realloc(board->moves,(board->size_moves+1)*sizeof(struct move_t));
    board->moves[board->size_moves]=move;
    board->size_moves++;

    board->current_positions[move.c] = move.m;
}

/*void board_free(struct board_t* board){
    //graph_free(board->graph);
    free(board->moves);
    free(board);
}*/

void board_free(struct board_t* board) {
    if (board) {
        free(board->moves);
        if (board->graph) {
            graph_free(board->graph);
            board->graph = NULL;
        }
        free(board);
    }
    board = NULL;
}


//on doit voir comment on peut generer wall sans que graph soit en parametre pour l utiliser dans la fct play
void generate_wall(struct edge_t e[2], struct board_t *board) {
    (void)board;
    e[0].fr = axial_to_index(1, 0, 5) ;
    e[0].to=  axial_to_index(0,1,5);
    e[1].to = axial_to_index(0,1,5);
    e[1].fr = axial_to_index(0,-1,5);
}


int is_invalid(struct move_t move, struct board_t* board) {
    if (!board || !board->graph) return 1;

    struct graph_t* graph = board->graph;
    vertex_t my_pos = graph->start[move.c];
    vertex_t opp_pos = graph->start[(move.c + 1) % NUM_PLAYERS];

    if (move.t == MOVE) {
        if (move.m >= graph->num_vertices) return 1;
        if (move.m == opp_pos) return 1;
        if (gsl_spmatrix_uint_get(graph->t, my_pos, move.m) == 7) return 1;
        if (gsl_spmatrix_uint_get(graph->t, my_pos, move.m) == 0) return 1;

        return 0;
    }

    else if (move.t == WALL) {
        for (int i = 0; i < 2; i++) {
            if (move.e[i].fr >= graph->num_vertices || move.e[i].to >= graph->num_vertices)
                return 1;
        }
        for (int i = 0; i < 2; i++) {
            if (gsl_spmatrix_uint_get(graph->t, move.e[i].fr, move.e[i].to) == 0)
                return 1;
        }
        if (move.e[0].fr != move.e[1].fr) return 1;

        enum dir_t d0 = gsl_spmatrix_uint_get(graph->t, move.e[0].fr, move.e[0].to);
        enum dir_t d1 = gsl_spmatrix_uint_get(graph->t, move.e[1].fr, move.e[1].to);

        if (abs((int)d0 - (int)d1) != 1 && abs((int)d0 - (int)d1) != 5)
            return 1;

        return 0; 
    }

    return 1; 
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

