    #include <stdio.h>
#include <dlfcn.h>
#include <getopt.h>
#include <stdlib.h>
#include "player.h"
#include "graph_functions.h"
#include "board.h"

//enum graph_type_t type;

struct player_t{
    enum player_color_t player_color;
    char const* (*get_player_name)();
    void (*initialize)(unsigned int, struct graph_t*);
    struct move_t (*play)(const struct move_t);
    void (*finalize)();
    void *library;
    vertex_t pos_actuel;
};

struct player_t players[NUM_PLAYERS];


void assert_dlsym(){
  char* error;
  if((error=dlerror())!=NULL){
    fputs(error,stderr);
    exit(1);
  }
}

void load_player(struct player_t *player, char *lib){
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

void first_step(int argc, char **argv){
    int j = 0;
    for (int i = 1; i < argc && j < NUM_PLAYERS; i++) {
        if (argv[i][0] != '-' && argv[i][0]   != '1' && argv[i][0] != '2' && argv[i][0] != '0') { 
            load_player(&players[j], argv[i]);
            j++;
        }
    } 
}
struct move_t *make_first_move() {

    struct move_t *first_move = malloc(sizeof(struct move_t));
    first_move->c = NO_COLOR;
    first_move->t = NO_TYPE;
    first_move->m = 0;
    first_move->e[0].fr = 0;
    first_move->e[0].to = 0;
    first_move->e[1].fr = 0;
    first_move->e[1].to = 0;
    return first_move;
}

int player_to_start(){
    return rand()%NUM_PLAYERS;
}

int main(int argc, char *argv[]){
    char *type_graph = NULL;
    int size_mesh = -1;     

    int opt;
    while ((opt = getopt(argc, argv, "m:t:")) != -1) {
        switch (opt) {
            case 'm':
                size_mesh = atoi(optarg);
                break;
            case 't':
                type_graph = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-m M] [-t T] player1.so player2.so\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Affichage des paramètres
    printf("Type de graphe: %s\n", type_graph);
    printf("Taille de la maille: %d\n", size_mesh);

    struct graph_t *graph1 = malloc(sizeof(struct graph_t)); // à compléter.....................!!!!!!
    struct graph_t *graph2 = malloc(sizeof(struct graph_t)); 
    struct graph_t *globalGraph = malloc(sizeof(struct graph_t));
    initialize_graph(graph1, 8, TRIANGULAR);
    initialize_graph(graph2, 8, TRIANGULAR);
    initialize_graph(globalGraph, 8, TRIANGULAR);

    struct move_t *first_move = make_first_move();
    

    if(syntax_test(argc) == -1){
        return EXIT_FAILURE;
    }
    int start_player = player_to_start();
    first_step(argc, argv);
    ///////////////this is the first player
    players[start_player].initialize(start_player, graph1);
    /*players[start_player].player_name = players[start_player].get_player_name();
    printf("First player:\t%s\n", players[start_player].player_name);*/
    const char *player_name1 = players[start_player].get_player_name();
    printf("First player:\t%s\n", player_name1);


    /////////////////////this is the second player 
    int next = (start_player + 1) % NUM_PLAYERS;
    players[next].initialize(next, graph2);
   /* players[next].player_name = players[next].get_player_name();
    printf("Second Player:\t%s\n",  players[next].player_name);*/
    const char *player_name2 = players[next].get_player_name();
    printf("First player:\t%s\n", player_name2);


    struct board_t *board = board_init();
    add_move_to_board(board, *first_move);



    

    printf("----------Starting Game----------\n");

    free(graph1) ; 
    free(graph2) ;
    free(globalGraph) ; 
    for (int i = 0; i < NUM_PLAYERS; i++) {
        //players[i].finalize();

        dlclose(players[i].library);
    }
    return 0;
}


