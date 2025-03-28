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

static struct player_t players[NUM_PLAYERS];


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

int syntax_test(int argc) {
    if(argc < 3) {
      printf("Error: Argument list is tooooo short\nUsage: ./install/server ./library1 ./library2\n");
      return -1;
    }
    return 0;
}

void first_step(){
    int j = 0;
    for (int i = 1; i < argc && j < NUM_PLAYERS; i++) {
        if (argv[i][0] != '-' && argv[i][0]   != '1' && argv[i][0] != '2' && argv[i][0] != '0') { 
            load_player(&players[j], argv[i]);
            j++;
        }
    } 
}
int player_to_start(){
    return rand()%NUM_PLAYERS;
}

int main(int argc, char *argv[]){
    struct graph_t graph = initilaize(5) // à compléter.....
    if(syntax_test(argc) == -1){
        return EXIT_FAILURE;
    }
    int start_player = player_to_start();
    first_step();
    ///////////////this is the first player
    players[start_player].initialize(start_player, , &g1);
    players[start_player].player_name = players[start_player].get_player_name();
    printf("First player:\t%s\n", players[start_player].player_name);

    /////////////////////this is the second player 
    int next = (start_player + 1) % NUM_PLAYERS;
    players[next].initialize(next, &g1);
    players[next].player_name = players[next].get_player_name();
    printf("Second Player:\t%s\n",  players[next].player_name);
}


