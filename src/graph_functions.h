#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include "graph.h"




struct graph_t* createGraph(unsigned int n, enum graph_type_t type) ;

void initialize_graph(struct graph_t *graph, unsigned int n , enum graph_type_t type ) ; 

void print_graph(struct graph_t *graph) ;

void free_graph(struct graph_t *graph)  ; 