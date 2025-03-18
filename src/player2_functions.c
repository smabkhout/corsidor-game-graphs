
#include "player2_functions.h"

int is_wall_ahead(struct player_t* player){
  if (ahead_position(player) == WALL)
    return 1;
  return 0;
}
