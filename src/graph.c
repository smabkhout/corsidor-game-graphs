#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_spmatrix.h>
#include "graph.h"




struct graph_t* createGraph(unsigned int n, enum graph_type_t type) {
  
  struct graph_t* graph = malloc(sizeof(struct graph_t));
  if (!graph) {
    fprintf(stderr, "Failed to allocate memory for graph\n");
    return NULL;
  }

  graph->type = type;
  graph->num_vertices = n;
  
  // Initialize the sparse matrix for adjacency
  gsl_spmatrix_uint* mat = gsl_spmatrix_uint_alloc(n, n);

  // Based on the graph type, we will add edges differently
  switch (type) {
    case TRIANGULAR:
      // Add edges for a triangular grid, each vertex connects to neighbors in 8 directions
      for (unsigned int i = 0; i < n; i++) {
        for (unsigned int d = FIRST_DIR; d <= LAST_DIR; d++) {
          // Assuming we have some logic here to determine the valid neighbors
          unsigned int j = i + d;  // For simplicity, just add some offset here
          if (j < n && j != i) {
            gsl_spmatrix_uint_set(mat, i, j, 1);  // Add edge i -> j
          }
        }
      }
      break;

    case CYCLIC:
      // Add edges for a cyclic graph (first connects to last)
      for (unsigned int i = 0; i < n; i++) {
        unsigned int j = (i + 1) % n;  // Wrap around to create the cycle
        gsl_spmatrix_uint_set(mat, i, j, 1);
        gsl_spmatrix_uint_set(mat, j, i, 1);  // As the cyclic graph is undirected
      }
      break;

    case HOLEY:
      // Add edges for a holey graph, for simplicity we add edges in 4 cardinal directions
      for (unsigned int i = 0; i < n; i++) {
        unsigned int j;
        for (unsigned int d = FIRST_DIR; d <= LAST_DIR; d++) {
          // Adjust this according to how you want to model the "holey" structure
          j = i + d; // Simple offset for demonstration
          if (j < n && j != i) {
            gsl_spmatrix_uint_set(mat, i, j, 1);
          }
        }
      }
      break;
  }

  // Compress the matrix into CSR format for efficient traversal
  gsl_spmatrix_uint* csr = gsl_spmatrix_uint_compress(mat, GSL_SPMATRIX_CSR);

  // Now we can iterate over the edges in CSR format
  for (unsigned int i = 0; i < csr->size1; i++) {
    for (unsigned int k = csr->p[i]; k < csr->p[i+1]; k++) {
      unsigned int j = csr->i[k];
      // Do something with the edge from i to j
    }
  }
  graph->t = csr;
  
  return graph;
}




void testGraph(struct graph_t* graph) {
  if (!graph) {
    printf("Graph is NULL.\n");
    return;
  }

  printf("Graph type: ");
  switch (graph->type) {
    case TRIANGULAR:
      printf("TRIANGULAR\n");
      break;
    case CYCLIC:
      printf("CYCLIC\n");
      break;
    case HOLEY:
      printf("HOLEY\n");
      break;
    default:
      printf("Unknown type\n");
      break;
  }

  printf("Number of vertices: %u\n", graph->num_vertices);
  printf("Number of edges: %u\n", graph->num_edges);

  // Iterate through the compressed sparse matrix (CSR)
  printf("Edges in the graph:\n");
  for (unsigned int i = 0; i < graph->t->size1; i++) {
    for (unsigned int k = graph->t->p[i]; k < graph->t->p[i + 1]; k++) {
      unsigned int j = graph->t->i[k];
      printf("Edge: %u -> %u\n", i, j);  // Print each edge from vertex i to j
    }
  }
}

int main() {
  unsigned int n = 10;  // Number of vertices in the graph
  struct graph_t* graph = createGraph(n, CYCLIC);

  if (graph) {
    testGraph(graph);  // Test and display the graph edges
  }

  return 0;
}
