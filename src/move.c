#include "move_functions.h"
#include <stdio.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>
#include <gsl/gsl_spblas.h>

//on doit voir comment on peut generer wall sans que graph soit en parametre pour l utiliser dans la fct play
void generate_wall(struct edge_t e[2]) {
    srand(1);
    e[0].fr = rand(); 
    e[0].to = rand();
    e[1].fr = rand();
    e[1].to = rand();
}






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

int can_place_wall(struct graph_t *graph, struct edge_t e[2]) {
    if (gsl_spmatrix_uint_get(graph->t, e[0].fr, e[0].to) > 0 && gsl_spmatrix_uint_get(graph->t, e[1].fr, e[1].to) > 0) {
        return 1; 
    }
    return 0; 
}




int is_empty_position( vertex_t n , vertex_t pos_other_player){
        if (pos_other_player == n){
            return 0;
        }
    return 1;
}

int can_move(struct graph_t *graph, vertex_t pos_player, vertex_t b, vertex_t pos_other_player) {
    if (is_empty_position(b, pos_other_player) && gsl_spmatrix_uint_get(graph->t, pos_player, b) > 0) {
        return 1;
    }
    return 0;
}




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




/* Apply a move and modify the graph accordingly */
void apply_move(struct move_t* move, struct graph_t* graph) {
    if (!move || !graph) {
        printf("Invalid move or graph!\n");
        return;
    }
    if (!is_valid_move(move, graph)) {
        printf("Invalid move! Move could not be applied.\n");
        return;
    }
    if (move->t == MOVE) {
        vertex_t new_pos = move->m;
        printf("Player %d moves to vertex %u\n", move->c, new_pos);
       // players[move->c]->pos_actuel = new_pos; ///La structure player_t et le tableau players doit être seulement utilisé dans server.c seulement!!!!!!!!!!!!!!!!
        // !!!! est ce qu on doit pas changer la position de player 
    } 
    else if (move->t == WALL) {
        // Process the wall move (modify the adjacency matrix)
        vertex_t v1 = move->e[0].fr;
        vertex_t v2 = move->e[0].to;
        vertex_t v3 = move->e[1].fr;
        vertex_t v4 = move->e[1].to;

        // Update adjacency matrix to remove the edges (add the wall)
        // Set the corresponding entries to 0 (no edge between these vertices)
        gsl_spmatrix_uint_set(graph->t, v1, v2, 0);  // Remove edge v1 -> v2
        gsl_spmatrix_uint_set(graph->t, v2, v1, 0);  // Remove edge v2 -> v1 (undirected graph)

        gsl_spmatrix_uint_set(graph->t, v3, v4, 0);  // Remove edge v3 -> v4
        gsl_spmatrix_uint_set(graph->t, v4, v3, 0);  // Remove edge v4 -> v3 (undirected graph)

        printf("Wall applied between vertices %u and %u, and %u and %u\n", v1, v2, v3, v4);
    }
}


void test_create_move() {
    struct edge_t edges[2] = {{0, 1}, {1, 2}};
    struct move_t move = create_move(BLACK, WALL, 0, edges);

    printf("Created Move:\n");
    printf("Player Color: %d\n", move.c);
    printf("Move Type: %d\n", move.t);
    printf("Vertex: %u\n", move.m);
    printf("Edge 1: (%u -> %u)\n", move.e[0].fr, move.e[0].to);
    printf("Edge 2: (%u -> %u)\n", move.e[1].fr, move.e[1].to);
}

void test_is_valid_move() {
    struct graph_t graph;
    graph.num_vertices = 4;  // Simple graph with 4 vertices

    // Create a valid move (player moves to vertex 2)
    struct move_t move1 = create_move(BLACK, MOVE, 2, NULL);
    int valid1 = is_valid_move(&move1, &graph);
    printf("Move 1 is valid: %d\n", valid1);  // Expect 1 (valid)

    // Create an invalid move (player moves to vertex 5, which is out of bounds)
    struct move_t move2 = create_move(WHITE, MOVE, 5, NULL);
    int valid2 = is_valid_move(&move2, &graph);
    printf("Move 2 is valid: %d\n", valid2);  // Expect 0 (invalid)

    // Create a valid wall move (remove edge between vertices 1 and 2)
    struct edge_t edges[2] = {{1, 2}, {2, 1}};
    struct move_t move3 = create_move(BLACK, WALL, 0, edges);
    int valid3 = is_valid_move(&move3, &graph);
    printf("Move 3 is valid: %d\n", valid3);  // Expect 1 (valid)

    // Create an invalid wall move (edge between vertices 5 and 6, out of bounds)
    struct edge_t edges_invalid[2] = {{5, 6}, {6, 5}};
    struct move_t move4 = create_move(WHITE, WALL, 0, edges_invalid);
    int valid4 = is_valid_move(&move4, &graph);
    printf("Move 4 is valid: %d\n", valid4);  // Expect 0 (invalid)
}

void test_apply_move() {
    struct graph_t graph;
    graph.num_vertices = 4;
    graph.start[BLACK] = 0;  // Player BLACK starts at vertex 0
    graph.start[WHITE] = 3;  // Player WHITE starts at vertex 3

    // Create and apply a valid player move (BLACK moves to vertex 2)
    struct move_t move1 = create_move(BLACK, MOVE, 2, NULL);
    printf("\nApplying Move 1 (Player BLACK moves to vertex 2):\n");
    apply_move(&move1, &graph);
    printf("Player BLACK new position: %u\n", graph.start[BLACK]);

    // Create and apply a wall move (WHITE blocks the edge between vertices 1 and 2)
    struct edge_t edges[2] = {{1, 2}, {2, 1}};
    struct move_t move2 = create_move(WHITE, WALL, 0, edges);
    printf("\nApplying Move 2 (Player WHITE blocks edge between 1 and 2):\n");
    apply_move(&move2, &graph);

    // Check the adjacency matrix after applying the wall move
    printf("Adjacency matrix after wall move:\n");
    for (unsigned int i = 0; i < graph.num_vertices; i++) {
        for (unsigned int j = 0; j < graph.num_vertices; j++) {
            printf("%u ", gsl_spmatrix_uint_get(graph.t, i, j));
        }
        printf("\n");
    }
}




int main() {
    printf("=== Test create_move ===\n");
    test_create_move();

    printf("\n=== Test is_valid_move ===\n");
    test_is_valid_move();

    printf("\n=== Test apply_move ===\n");
    test_apply_move();

    return 0;
}
