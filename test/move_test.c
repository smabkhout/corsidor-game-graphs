#include "move_test.h"
#include <stdio.h>
#include <assert.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>


void test_create_move() {
    struct edge_t edges[2] = {{0, 1}, {1, 2}};
    struct move_t move = create_move(BLACK, WALL, 0, edges);
    assert(move.c == BLACK);
    assert(move.t == WALL);
    assert(move.m == 0);
    assert(move.e[0].fr == 0 && move.e[0].to == 1);
    assert(move.e[1].fr == 1 && move.e[1].to == 2);
    printf("test_create_move passed.\n");
}


void test_is_empty_position() {
    assert(is_empty_position(2, 3) == 1);
    assert(is_empty_position(4, 4) == 0);
    printf("test_is_empty_position passed.\n");
}

void test_is_connected() {
    struct graph_t graph;
    graph.num_vertices = 3;
    graph.t = gsl_spmatrix_uint_alloc(3, 3);
    gsl_spmatrix_uint_set(graph.t, 0, 1, 1);
    gsl_spmatrix_uint_set(graph.t, 1, 2, 1);
    assert(is_connected(&graph, 0, 1) == 1);
    assert(is_connected(&graph, 1, 2) == 1);
    assert(is_connected(&graph, 0, 2) == 0);
    gsl_spmatrix_uint_free(graph.t);
    printf("test_is_connected passed.\n");
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


void test_dijkstra() {
    struct graph_t graph;
    graph.num_vertices = 3;
    graph.t = gsl_spmatrix_uint_alloc(3, 3);
    gsl_spmatrix_uint_set(graph.t, 0, 1, 1);
    gsl_spmatrix_uint_set(graph.t, 1, 2, 1);
    
    int d[3];
    int prev[3];
    dijistra(&graph, 0, 2, d, prev);
    
    assert(d[2] == 2);
    assert(prev[2] == 1);
    assert(prev[1] == 0);
    
    gsl_spmatrix_uint_free(graph.t);
    printf("test_dijkstra passed.\n");
}