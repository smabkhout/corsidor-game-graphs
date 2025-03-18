#include "player_functions.h"

char const* get_player_name()
{
    char* name = "Abdelkader";
    return name;
}

void initialize(unsigned int player_id, struct graph_t* graph)
{
    (void) player_id;
    (void) graph;
}

struct move_t play(const struct move_t previous_move)
{
    return previous_move;
}

void finalize()
{
    int a = 0;
    (void) a;
}