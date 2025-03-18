#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include "graph.h"



int main() {
    struct graph_t example_graph;
    example_graph.type = TRIANGULAR;
    example_graph.num_vertices = 5;
    example_graph.num_edges = 0;
    example_graph.t = gsl_spmatrix_uint_alloc(5, 5);
    example_graph.num_objectives = 2;
    example_graph.objectives = malloc(2 * sizeof(vertex_t));
    if (!example_graph.objectives) {
        fprintf(stderr, "Erreur d'allocation mémoire\n");
        return EXIT_FAILURE;
    }
    example_graph.objectives[0] = 1;
    example_graph.objectives[1] = 3;
    
    printf("Graphe de type %d avec %u sommets.\n", example_graph.type, example_graph.num_vertices);
    
    free(example_graph.objectives);
    gsl_spmatrix_uint_free(example_graph.t);
    return EXIT_SUCCESS;
}
