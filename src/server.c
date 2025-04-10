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
      fprintf(stderr, "Error opening library %s: %s\n", lib, dlerror());
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

/*void first_step(int argc, char **argv){
    int j = 0;
    for (int i = 1; i < argc && j < NUM_PLAYERS; i++) {
        if (argv[i][0] != '-' && argv[i][0]   != '1' && argv[i][0] != '2' && argv[i][0] != '0' && argv[i][0] != '5') { 
            load_player(&players[j], argv[i]);
            j++;
        }
    } 
}*/
void first_step(int argc, char **argv) {
    int j = 0;
    for (int i = optind; i < argc && j < NUM_PLAYERS; i++) {
        load_player(&players[j], argv[i]);
        j++;
    }
}

const char* move_type_to_string(enum move_type_t type) {
    switch (type) {
        case NO_TYPE: return "NO_TYPE";
        case WALL: return "WALL";
        case MOVE: return "MOVE";
        default: return "UNKNOWN_TYPE";
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
    int size_mesh = -1;
    char *type_graph = NULL;
    int max_turns = -1;

    int opt;
    while ((opt = getopt(argc, argv, "m:t:M:")) != -1) {
        switch (opt) {
            case 'm':
                size_mesh = atoi(optarg);
                break;
            case 't':
                type_graph = optarg;
                break;
            case 'M': max_turns = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-m M] [-t T] [-M NB] player1.so player2.so\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (argc - optind != NUM_PLAYERS) {
        fprintf(stderr, "Error: You must provide exactly two player libraries.\n");
        fprintf(stderr, "Usage: %s [-m M] [-t T] [-M NB] player1.so player2.so\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Type de graphe: %s\n", type_graph);
    printf("Taille de la maille: %d\n", size_mesh);
    first_step(argc, argv);

    //struct graph_t *graph1 = createGraph(7, TRIANGULAR); 
    //struct graph_t *graph2 = createGraph(7, TRIANGULAR); 
    struct graph_t *globalGraph = createGraph(size_mesh, TRIANGULAR); 
    struct board_t *board = board_init();
    board->graph = globalGraph ; 
    struct move_t *first_move = make_first_move();
    

    if(syntax_test(argc) == -1){
        return EXIT_FAILURE;
    }


    int start_player = player_to_start();
    int current_player = start_player;
    int other_player = (start_player + 1) % NUM_PLAYERS;

    players[start_player].initialize(start_player, globalGraph);
    players[other_player].initialize(other_player, globalGraph);

    printf("First player: %s\n", players[start_player].get_player_name());
    printf("Second player: %s\n", players[other_player].get_player_name());

    int winner = -1;
    int turn_count = 0;
    int max_turns = 5;

    printf("----------Starting Game----------\n");


    struct board_t *board = board_init();
    board->graph = globalGraph ; 
    add_move_to_board(board, *first_move);



    printf("The size of the board is: %d vertices \n" , board->graph->num_vertices) ; 

    printf("----------Starting Game----------\n");


    struct move_t current_move = *first_move;
    printf("The server did the first move : %s\n", move_type_to_string(current_move.t));
    printf("In the vertex %d \n", current_move.m);
    printf("The number of moves played so far is: %d\n", board->size_moves);
    
    while (winner == -1 && turn_count < max_turns) {
        struct move_t move = players[current_player].play(current_move);
        if (is_invalid(move,board)){
            printf("🤖 Player %s executed an illegal move. Did they even read the rules? RIP\n", players[current_player].get_player_name());

            winner = (current_player + 1) % NUM_PLAYERS;
            break;
        }

        printf("Turn %d: Player %s plays %s to vertex %lu\n", turn_count,
               players[current_player].get_player_name(), move_type_to_string(move.t), move.m);

        if (move.t != NO_TYPE) {
            add_move_to_board(board, move);
            current_move = move;
            current_player = (current_player + 1) % NUM_PLAYERS;
            other_player = (current_player + 1) % NUM_PLAYERS;
            turn_count++;

            for (unsigned int i = 0; i < globalGraph->num_objectives; i++) {
                if (globalGraph->start[current_move.c] == globalGraph->objectives[i]) {
                    winner = current_move.c;
                    break;
                }
            }
        } else {
            printf("Invalid move by player %s — they lose!\n", players[current_player].get_player_name());
            winner = (current_player + 1) % NUM_PLAYERS; 
            break;
        }
    }

    if (winner >= 0) {
        printf("\n🎉 Player %s wins the game!\n", players[winner].get_player_name());
    } else {
        printf("\n⏱️  Game ended in a draw (max turns reached)\n");
    }
    printf("----------The END----------");

    //free(globalGraph) ; 
    board_free(board);
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].finalize();

        dlclose(players[i].library);
    }
    return 0;
}


