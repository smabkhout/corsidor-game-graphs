#include "board.h"
#include "graph_functions.h"
#include "move.h"
#include "move2.h"
#include "player.h"
#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

struct player_t {
  enum player_color_t player_color;
  char const *(*get_player_name)();
  void (*initialize)(unsigned int, struct graph_t *);
  struct move_t (*play)(const struct move_t);
  void (*finalize)();
  void *library;
  vertex_t pos_actuel;
};

struct player_t players[NUM_PLAYERS];

void assert_dlsym() {
  char *error;
  if ((error = dlerror()) != NULL) {
    fputs(error, stderr);
    exit(1);
  }
}

void load_player(struct player_t *player, char *lib) {
  void *library = dlopen(lib, RTLD_LAZY);
  if (!library) {
    fprintf(stderr, "Error opening library %s: %s\n", lib, dlerror());
    exit(1);
  }

  player->library = library;
  player->get_player_name =
      (char const *(*)(void))dlsym(library, "get_player_name");
  assert_dlsym();
  player->initialize =
      (void (*)(unsigned int, struct graph_t *))dlsym(library, "initialize");
  assert_dlsym();
  player->play = (struct move_t (*)(const struct move_t))dlsym(library, "play");
  assert_dlsym();
  player->finalize = (void (*)(void))dlsym(library, "finalize");
  assert_dlsym();
}

int syntax_test(int argc) {
  if (argc < 3) {
    printf("Error: Argument list is too short\nUsage: ./install/server "
           "./library1 ./library2\n");
    return -1;
  }
  return 0;
}

void first_step(int argc, char **argv) {
  int j = 0;
  for (int i = optind; i < argc && j < NUM_PLAYERS; i++) {
    load_player(&players[j], argv[i]);
    j++;
  }
}

const char *move_type_to_string(enum move_type_t type) {
  switch (type) {
  case NO_TYPE:
    return "NO_TYPE";
  case WALL:
    return "WALL";
  case MOVE:
    return "MOVE";
  default:
    return "UNKNOWN_TYPE";
  }
}

struct move_t *make_first_move() {
  struct move_t *first_move = malloc(sizeof(struct move_t));
  first_move->c = 0;
  first_move->t = MOVE;
  first_move->m = 0;
  first_move->e[0].fr = 0;
  first_move->e[0].to = 0;
  first_move->e[1].fr = 0;
  first_move->e[1].to = 0;
  return first_move;
}
struct move_t *makee_first_move() {
  struct move_t *first_move = malloc(sizeof(struct move_t));
  first_move->c = 1;
  first_move->t = MOVE;
  first_move->m = 60 ;

  return first_move;
}

int player_to_start() { return rand() % NUM_PLAYERS; }

int main(int argc, char *argv[]) {
  int size_mesh = -1;
  char *type_graph = NULL;
  int max_turns = -1;
  int seed = 150;
  int affichage = 0;

  int opt;
  while ((opt = getopt(argc, argv, "m:t:M:s:v")) != -1) {
    switch (opt) {
    case 'm':
      size_mesh = atoi(optarg);
      break;
    case 't':
      type_graph = optarg;
      break;
    case 'M':
      max_turns = atoi(optarg);
      break;
    case 's':
      seed = atoi(optarg);
      break;
    case 'v':
      affichage = 1;
      break;
    default:
      fprintf(stderr,
              "Usage: %s [-m M] [-t T] [-M NB] [-v] [-s seed] libplayer1.so "
              "libplayer2.so\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  srand(seed);

  if (argc - optind != NUM_PLAYERS) {
    fprintf(stderr, "Error: You must provide exactly two player libraries.\n");
    fprintf(stderr,
            "Usage: %s [-m M] [-t T] [-M NB] [-v] [-s seed] libplayer1.so "
            "libplayer2.so\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Type de graphe: %s\n", type_graph);
  printf("Taille de la maille: %d\n", size_mesh);
  first_step(argc, argv);

  // board->graph = malloc(sizeof(struct graph_t));
  // copy_graph(board->graph, globalGraph);

  struct move_t *first_move = make_first_move();
  struct move_t *first_move2 = makee_first_move();

  if (syntax_test(argc) == -1) {
    return EXIT_FAILURE;
  }

  int start_player = player_to_start();
  int current_player = start_player;
  int other_player = (start_player + 1) % NUM_PLAYERS;

  struct graph_t *g1 = createGraph(size_mesh, TRIANGULAR);
  struct graph_t *g2 = createGraph(size_mesh, TRIANGULAR);
  int returnHome[2] = {0 ,0} ;
  int (*visitedObjectif)[2] = malloc(g1->num_objectives* sizeof(*visitedObjectif));  
  for (unsigned int i = 0 ; i<g1->num_objectives ; i++){
        visitedObjectif[i][0] = 0 ;
        visitedObjectif[i][1] = 0 ;
  }
  vertex_t  start[2] = {g1->start[0], g2->start[1]};

   

  struct graph_t *graphs[2] = {g1, g2};
  // struct graph_t *globalGraph = g1;
  struct board_t *board = board_init();
  board->graph = g1;

  vertex_t last_positions[2];
  vertex_t current_positions[2];

  players[current_player].initialize(current_player, graphs[current_player]);
  players[other_player].initialize(other_player, graphs[other_player]);

  players[current_player].pos_actuel = board->graph->start[current_player];
  players[other_player].pos_actuel = board->graph->start[other_player];

  last_positions[current_player] = board->graph->start[current_player];
  last_positions[other_player] = board->graph->start[other_player];

  current_positions[current_player] = board->graph->start[current_player];
  current_positions[other_player] = board->graph->start[other_player];

  printf("First player: %s\n", players[start_player].get_player_name());
  printf("Second player: %s\n", players[other_player].get_player_name());

  int winner = -1;
  int turn_count = 0;

  add_move_to_board(board, *first_move);
  printf("The size of the board is: %d vertices \n",
         board->graph->num_vertices);
  printf("----------Starting Game----------\n");

  struct move_t current_move = *first_move;
  printf("The server did the first move : %s\n",
         move_type_to_string(current_move.t));
  printf("In the vertex %d \n", current_move.m);
  if (affichage)
    print_hex_grid(board->graph);
  printf("The number of moves played so far is: %d\n", board->size_moves);
  struct move_t moves_act[2] = {current_move, *first_move2};

  while (winner == -1 && turn_count < max_turns) {
    // printf("psition actuelle du joueur 1 : %d  , position precedente %d  \n"
    // , current_positions[0] , last_positions[0] ) ; printf("psition actuelle
    // du joueur 2 : %d  , position precedente %d \n" , current_positions[1] ,
    // last_positions[1] ) ;
   
    int all_objectives_are_visited = 1;
    for ( unsigned int i = 0 ; i<graphs[current_player]->num_objectives ; i++){
        if (visitedObjectif[i][current_player] == 0 ){
            all_objectives_are_visited = 0 ;
            
        }
    }

    if (all_objectives_are_visited && returnHome[current_player] == 1) {
        //stop the game the winner is the current player
        winner = current_player;
        printf("Player %s visited all objectives and returned home!\n",
               players[current_player].get_player_name());
        break;
    }

    struct player_tt *current_player_ptr = malloc(sizeof(struct player_tt));
    current_player_ptr->position = players[current_player].pos_actuel;
    current_player_ptr->c = players[current_player].player_color;
    // current_player_ptr.walls = players[current_player].walls;
    current_player_ptr->last_position = last_positions[current_player];

    // l'autre joueur
    struct player_tt *other_player_ptr = malloc(sizeof(struct player_tt));
    other_player_ptr->position = players[other_player].pos_actuel;
    other_player_ptr->c = players[other_player].player_color;
    // other_player_ptr.walls = players[other_player].walls;
    other_player_ptr->last_position = last_positions[other_player];

    struct move_t move =players[current_player].play(moves_act[other_player]);
    graphs[current_player]->start[current_player] = move.m; // stocker la nouvelle positions de current player dans les deux
                // graphes des joueurs
    graphs[other_player]->start[current_player] = move.m;
    moves_act[current_player] = move;
    last_positions[current_player] = current_positions[current_player];
    current_positions[current_player] = move.m;
    if (!valid_move(board->graph, current_player_ptr, move.m,
                    moves_act[other_player].m)) {
      int id = move.c;
      if (id != current_player) {
        printf("Player %s with id %d returned a move with id: %d\n",
               players[current_player].get_player_name(), current_player,
               move.c);
      }
      if (affichage)
        print_hex_grid(board->graph);
      printf("🤖 Player %s executed an illegal move of type %s. Did they even "
             "read the rules? RIP\n",
             players[current_player].get_player_name(),
             move_type_to_string(move.t));
      printf("from %d to %d ", last_positions[current_player], move.m);
      winner = (current_player + 1) % NUM_PLAYERS;
      free(current_player_ptr);
      free(other_player_ptr);
      break;
    }
    // board->graph->start[current_player] = players[current_player].pos_actuel;
    if (affichage)
      print_hex_grid(board->graph);

    printf("Turn %d: Player %s plays %s from %u to vertex %u, his id is: %d\n",
           turn_count, players[current_player].get_player_name(),
           move_type_to_string(move.t), players[current_player].pos_actuel,
           move.m, current_player);
    last_positions[current_player] =players[current_player].pos_actuel; // on stocke l'ancienne position et apres la nouvelle
    players[current_player].pos_actuel = move.m;
    moves_act[current_player] = move;
    if (move.t != NO_TYPE) {
      add_move_to_board(board, move);
      current_move = move;
      current_player = (current_player + 1) % NUM_PLAYERS;
      other_player = (current_player + 1) % NUM_PLAYERS;
      turn_count++;
      /*
                  if (g1->start[current_move.c] ==
         board->current_positions[current_move.c]) { winner = current_move.c;
                      free(current_player_ptr);
                      free(other_player_ptr);
                      break;
                      }
      */
    } else {
      printf("Invalid move by player %s — they lose!\n",
             players[current_player].get_player_name());
      winner = (current_player + 1) % NUM_PLAYERS;
      free(current_player_ptr);
      free(other_player_ptr);
      break;
    }
    // print_hex_grid(board->graph);

    for (unsigned int i = 0; i <graphs[current_player]->num_objectives; i++) {
      if (g1->objectives[i] == current_positions[current_player]) {
        visitedObjectif[i][current_player] = 1;
        printf("Player %s visited objective %d\n",
               players[current_player].get_player_name(), i);
        break;
      }
    }
    printf("all_objectives_are_visited = %d\n", all_objectives_are_visited);
    if ((all_objectives_are_visited==1) && (move.m == start[current_player])) {
      returnHome[current_player] = 1;
      printf("Player %s returned home!\n",
             players[current_player].get_player_name());
    }

    free(current_player_ptr);
    free(other_player_ptr);

  }

  if (winner >= 0) {
    printf("\n🎉 Player %s wins the game!\n",
           players[winner].get_player_name());
  } else {
    printf("\n⏱️  Game ended in a draw (max turns reached)\n");
  }
  printf("----------The END----------\n");

  // Libération des ressources
  for (int i = 0; i < NUM_PLAYERS; i++) {
    players[i].finalize();
    dlclose(players[i].library);
  }
  // graph_free(globalGraph);
  // board_free(board);
  free(board->moves);
  free(board);
  free(first_move);
  free(first_move2);
  free(visitedObjectif);

  return 0;
}
