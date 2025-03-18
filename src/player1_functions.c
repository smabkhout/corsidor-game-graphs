#include "player1_functions.h"

struct move_t inverted_move(struct move_t move)
{
    struct move_t inverted = {.c = move.c, .t = 2, .m = move.m, .e = move.e};
    if (move.t)
}

struct move_t* possible_moves(struct move_t previous, struct graph_t* graph)
{

}

struct move_t move_to_obj(struct move_t previous, struct graph_t* graph)
{
    
}