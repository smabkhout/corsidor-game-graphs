#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include "graph.h"




struct graph_t* createGraph(unsigned int n, enum graph_type_t type);

void initialize_graph(struct graph_t *graph, unsigned int n , enum graph_type_t type ); 

void print_graph(struct graph_t *graph);
size_t find(void* const s[], size_t size, void* c);
void graph_generate_T(int m, struct graph_t *g, int (*in_hexagon)(int l, int c, int m));

void free_graph(struct graph_t *graph) ; 