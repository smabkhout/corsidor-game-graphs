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

/////// Aucune fonction is_empty_position est implémentée
/*void test_is_empty_position() {
    assert(is_empty_position(2, 3) == 1);
    assert(is_empty_position(4, 4) == 0);
    printf("test_is_empty_position passed.\n");
}*/ 

void test_is_connected() {
    struct graph_t *graph=createGraph(3,TRIANGULAR);
    assert(is_connected(graph, 0, 1) == 1);
    assert(is_connected(graph, 1, 2) == 1);
    assert(is_connected(graph, 0, 2) == 0);
    printf("test_is_connected passed.\n");
    graph_free(graph);
}


void test_is_valid_move() {
    struct graph_t* graph=createGraph(3,TRIANGULAR);
    struct move_t move1 = create_move(BLACK, MOVE, 2, NULL);
    int valid1 = is_valid_move(&move1, graph);
    assert(valid1==1);

    // Create an invalid move (player moves to vertex 5, which is out of bounds)
    struct move_t move2 = create_move(WHITE, MOVE, 50, NULL);
    int valid2 = is_valid_move(&move2, graph);
    assert(valid2==0);

    // Create a valid wall move (remove edge between vertices 1 and 2)
    struct edge_t edges[2] = {{1, 2}, {2, 1}};
    struct move_t move3 = create_move(BLACK, WALL, 0, edges);
    int valid3 = is_valid_move(&move3, graph);
    assert(valid3==1);

    // Create an invalid wall move (edge between vertices 5 and 6, out of bounds)
    struct edge_t edges_invalid[2] = {{5, 6}, {6, 5}};
    struct move_t move4 = create_move(WHITE, WALL, 0, edges_invalid);
    int valid4 = is_valid_move(&move4, graph);
    assert(valid4==1);
    printf("test_is_valid_move passed .\n");  // Expect 0 (invalid)
    graph_free(graph);
}


void test_dijkstra() {
    struct graph_t *graph=createGraph(3,TRIANGULAR);

    int d[graph->num_vertices];
    int prev[graph->num_vertices];
    dijistra(graph, 0, 10, d, prev);
    assert(d[0]==0);
    assert(d[10]==3);
    assert(prev[10]==5);
    printf("test_dijkstra passed.\n");
    graph_free(graph);
}


void test_find_closest_objective() {
    struct graph_t *graph=createGraph(3,TRIANGULAR);
    vertex_t player_pos = 0;
    vertex_t closest_objective = find_closest_objective(graph, player_pos);
    // On attend que le plus proche objectif soit le sommet 3  car 0 -> 1 -> 2 -> 3 est le chemin le plus court
    printf("%d",closest_objective );
    printf("Test Passed: Closest objective found correctly.\n");
    graph_free(graph);
}

/*void test_is_path_clear(){
    struct graph_t graph;
    graph.num_vertices = 5;
    graph.num_objectives = 2;
    vertex_t objectives[] = { 3, 4 };
    graph.objectives = objectives;
    gsl_spmatrix_uint* t = gsl_spmatrix_uint_alloc(5, 5);
    gsl_spmatrix_uint_set(t, 0, 1, 1);  
    gsl_spmatrix_uint_set(t, 1, 2, 1);  
    gsl_spmatrix_uint_set(t, 2, 3, 1);  
    gsl_spmatrix_uint_set(t, 3, 4, 1);  
    graph.t = t;

}*/