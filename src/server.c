#include <stdio.h>
#include <dlfcn.h>
#include <getopt.h>
#include "player.h"
#include "move.h"
#include "graph.h"

struct player_t player
{
    enum player_color_t player_color;
    char const* (*get_player_name)();
    void (*initialize)(unsigned int, struct graph_t*);
    struct move_t (*play)(const struct move_t previous_move);
    void (*finalize)();
    void *library;
    vertex_t pos_actuel;
};

void assert_dlsym(){
  char* error;
  if((error=dlerror())!=NULL){
    fputs(error,stderr);
    exit(1);
  }
}

void load_player(struct player_t player, char *lib){
    void *library=dlopen(lib,RTLD_LAZY);
    if(!library){
      fputs(dlerror(),stderr);
      exit(1);
    }

    player->library=library;
    player->get_player_name=dlsym(library, "get_player_name");
    assert_dlsym();
    player->initialize=dlsym(library, "initialize");
    assert_dlsym();
    player->play=dlsym(library, "play");
    assert_dlsym();
    player->finalize=dlsym(library, "finalize");
    assert_dlsym();

  
}

