#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include "graph.h"


#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

struct graph_t* createGraph(unsigned int n, enum graph_type_t type) {
    struct graph_t* graph = (struct graph_t*)malloc(sizeof(struct graph_t));
    if (!graph) {
        perror("Failed to allocate memory for graph");
        exit(1);
    }

    graph->num_vertices = n;
    graph->num_edges = 0;
    graph->type = type;
    graph->t = gsl_spmatrix_uint_alloc(n, n);

    // Initialisation de la matrice d'adjacence
    for (unsigned int i = 0; i < n; i++) {
        for (unsigned int j = 0; j < n; j++) {
            gsl_spmatrix_uint_set(graph->t, i, j, 0); // Pas d'arêtes par défaut
        }
    }

    // Construction des arêtes en fonction du type de graphe
    if (type == TRIANGULAR) {
        // Graphe triangulaire
        for (unsigned int i = 0; i < n - 1; i++) {
            for (unsigned int j = i + 1; j < n; j++) {
                gsl_spmatrix_uint_set(graph->t, i, j, 1);  // Ajouter une arête entre i et j
                gsl_spmatrix_uint_set(graph->t, j, i, 1);  // Graphe non orienté
                graph->num_edges++;
            }
        }
    } else if (type == CYCLIC) {
        // Graphe cyclique (cercle)
        for (unsigned int i = 0; i < n - 1; i++) {
            gsl_spmatrix_uint_set(graph->t, i, i + 1, 1);
            gsl_spmatrix_uint_set(graph->t, i + 1, i, 1);
        }
        // Relier le dernier sommet au premier pour former un cycle
        gsl_spmatrix_uint_set(graph->t, n - 1, 0, 1);
        gsl_spmatrix_uint_set(graph->t, 0, n - 1, 1);
        graph->num_edges = n;
    } else if (type == HOLEY) {
        // Graphe avec des trous (pas d'arêtes entre certains sommets)
        for (unsigned int i = 0; i < n - 1; i++) {
            for (unsigned int j = i + 1; j < n; j++) {
                if (rand() % 2 == 0) {  // Par exemple, on ajoute une arête aléatoirement
                    gsl_spmatrix_uint_set(graph->t, i, j, 1);
                    gsl_spmatrix_uint_set(graph->t, j, i, 1);
                    graph->num_edges++;
                }
            }
        }
    }

    return graph;
}


void initialize(struct graph_t *graph, unsigned int n) {
    // Créer un graphe de type TRIANGULAR avec n sommets
    *graph = *createGraph(n, HOLEY);

    // Initialiser les objectifs et les positions des joueurs
    graph->num_objectives = 1;
    graph->objectives = (vertex_t*)malloc(sizeof(vertex_t));
    graph->objectives[0] = n / 2;  // Placer l'objectif au centre du graphe (par exemple)

    // Initialiser les positions de départ des joueurs
    graph->start[0] = 0;  // Premier joueur au sommet 0
    graph->start[1] = n - 1;  // Deuxième joueur au dernier sommet
}


void print_graph(struct graph_t *graph) {
    printf("Graph Type: %d\n", graph->type);
    printf("Number of vertices: %u\n", graph->num_vertices);
    printf("Number of edges: %u\n", graph->num_edges);
    
    // Affichage de la matrice d'adjacence
    printf("Adjacency Matrix:\n");
    for (unsigned int i = 0; i < graph->num_vertices; i++) {
        for (unsigned int j = 0; j < graph->num_vertices; j++) {
            printf("%d ", gsl_spmatrix_uint_get(graph->t, i, j));
        }
        printf("\n");
    }

    // Affichage des positions de départ des joueurs
    printf("Starting positions:\n");
    for (int i = 0; i < NUM_PLAYERS; i++) {
        printf("Player %d starts at vertex %u\n", i, graph->start[i]);
    }

    // Affichage des objectifs
    printf("Objectives:\n");
    for (unsigned int i = 0; i < graph->num_objectives; i++) {
        printf("Objective %u: vertex %u\n", i, graph->objectives[i]);
    }
}

void free_graph(struct graph_t *graph) {
    gsl_spmatrix_uint_free(graph->t);
    if (graph->objectives) {
        free(graph->objectives);
    }
    free(graph);
}




int main() {
  unsigned int n = 10;  // Number of vertices in the graph
  struct graph_t* graph = createGraph(n, HOLEY);
  initialize(graph , n ) ; 

  if (graph) {
    print_graph(graph) ; 
  }

  return 0;
}
