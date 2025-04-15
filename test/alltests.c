#include <stdio.h>
#include "graph_test.h"
#include "move_test.h"

int main(){
    test_create_Graph();
    test_create_move();
    test_is_empty_vertice() ;
    test_is_connected();
    test_is_valid_move() ;
    test_dijkstra();
    test_calculate_dist_objectives() ;
    test_calculate_total_dist();
    test_next_permutation() ;
   // test_TSP() ;
    test_find_closest_objective() ;
    
    return 0;
}