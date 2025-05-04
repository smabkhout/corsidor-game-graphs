#include "test_player.h"
#include <stdio.h>
#include <assert.h>
#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_spmatrix_uint.h>

struct graph_t* make_test_graph(int size) {
    struct graph_t* g = malloc(sizeof(struct graph_t));
    g->num_vertices = size;
    g->num_edges = 0;
    g->type = TRIANGULAR;
    g->num_objectives = 0;
    g->start[0] = 0;
    g->start[1] = 1;
    g->t = gsl_spmatrix_uint_alloc(size, size); 
    g->objectives = NULL;
    return g;
}

void test_neighbors_basic() {
    struct graph_t* g = make_test_graph(3);
    gsl_spmatrix_uint_set(g->t, 0, 1, 1);
    gsl_spmatrix_uint_set(g->t, 0, 2, 1);

    vertex_t out[4];
    int count = get_neighbors(g, 0, out, 4);

    assert(count == 2);
    assert((out[0] == 1 && out[1] == 2) || (out[0] == 2 && out[1] == 1));

    graph_free(g);
    printf("test_neighbors_basic ✅\n");
}

void test_max_out_zero() {
    struct graph_t* g = make_test_graph(3);
    gsl_spmatrix_uint_set(g->t, 0, 1, 1);

    vertex_t out[1];
    int count = get_neighbors(g, 0, out, 0);

    assert(count == 0);

    graph_free(g);
    printf("test_max_out_zero ✅\n");
}

void test_no_neighbors() {
    struct graph_t* g = make_test_graph(3);

    vertex_t out[2];
    int count = get_neighbors(g, 1, out, 2);

    assert(count == 0);

    graph_free(g);
    printf("test_no_neighbors ✅\n");
}

void test_limit_max_out() {
    struct graph_t* g = make_test_graph(4);
    gsl_spmatrix_uint_set(g->t, 0, 1, 1);
    gsl_spmatrix_uint_set(g->t, 0, 2, 1);
    gsl_spmatrix_uint_set(g->t, 0, 3, 1);

    vertex_t out[2];
    int count = get_neighbors(g, 0, out, 2);

    assert(count == 2);
    assert(out[0] != out[1]);

    graph_free(g);
    printf("test_limit_max_out ✅\n");
}

void test_invalid_vertex() {
    struct graph_t* g = make_test_graph(3);
    vertex_t out[2];

    int count = get_neighbors(g, 999, out, 2);
    assert(count == 0); 

    graph_free(g);
    printf("test_invalid_vertex ✅\n");
}

/*void test_play(){
    struct graph_t *g = createGraph(5, TRIANGULAR);
    struct move_t prev = { .t = MOVE, .c = 0 };
    initialize(0, g); 
    struct move_t m = play(prev);
    printf("Type de coup : %d\n", m.t);
    if (m.t == MOVE) {
        printf("Déplacement vers %u\n", m.m);
    }
    finalize();
    graph_free(g);

}
*/