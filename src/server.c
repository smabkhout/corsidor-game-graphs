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
  void    *library;
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

  player->library         = library;
  player->get_player_name = (char const *(*)(void))dlsym(library, "get_player_name");
  assert_dlsym();
  player->initialize = (void (*)(unsigned int, struct graph_t *))dlsym(library, "initialize");
  assert_dlsym();
  player->play = (struct move_t(*)(const struct move_t))dlsym(library, "play");
  assert_dlsym();
  player->finalize = (void (*)(void))dlsym(library, "finalize");
  assert_dlsym();
}

int syntax_test(int argc) {
  if (argc < 3) {
    printf(
        "Error: Argument list is too short\nUsage: ./install/server "
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
  first_move->c             = 0;
  first_move->t             = NO_TYPE;
  first_move->m             = 0;
  first_move->e[0].fr       = 0;
  first_move->e[0].to       = 0;
  first_move->e[1].fr       = 0;
  first_move->e[1].to       = 0;
  return first_move;
}
struct move_t *makee_first_move() {
  struct move_t *first_move = malloc(sizeof(struct move_t));
  first_move->c             = 1;
  first_move->t             = MOVE;
  first_move->m             = 18;

  return first_move;
}

int player_to_start() {
  return rand() % NUM_PLAYERS;
}

void print_walls(const struct graph_t *g) {
  printf("Murs actuels dans le graphe du serveur :\n");
  for (size_t i = 0; i < g->num_vertices; ++i) {
    for (size_t j = i + 1; j < g->num_vertices; ++j) {
      unsigned int val = gsl_spmatrix_uint_get(g->t, i, j);
      if (val == 7) {
        printf("Mur entre %lu et %lu\n", i, j);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  int   size_mesh  = -1;
  char *type_graph = NULL;
  int   max_turns  = -1;
  int   seed       = 150;
  int   affichage  = 0;

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
  int g_type;

  if (type_graph == NULL) {
    fprintf(stderr, "Type de graphe non spécifié, je tombe sur TRIANGULAR\n");
    g_type     = TRIANGULAR;
    type_graph = "TRIANGULAR";
  } else if (size_mesh < 0) {
    fprintf(stderr, "Taille de la maille non spécifiée, je tombe sur 3\n");
    size_mesh = 3;
  }

  // Valeurs par défaut si pas d’option -t ou -m
  if (type_graph == NULL) {
    fprintf(stderr, "Type de graphe non spécifié, je tombe sur TRIANGULAR\n");
    g_type     = TRIANGULAR;
    type_graph = "T";
  }
  if (size_mesh < 0) {
    fprintf(stderr, "Taille de la maille non spécifiée, je tombe sur 3\n");
    size_mesh = 3;
  }

  switch (type_graph[0]) {
    case 'T':
    case 't':
      if (strcmp(type_graph, "T") == 0)
        g_type = TRIANGULAR;
      else if (strcmp(type_graph, "TR") == 0)
        g_type = 4;  // TRIANGULAR_RANDOM
      else
        goto unknown;
      break;

    case 'C':
    case 'c':
      if (strcmp(type_graph, "C") == 0)
        g_type = CYCLIC;
      else
        goto unknown;
      break;

    case 'H':
    case 'h':
      if (strcmp(type_graph, "H") == 0)
        g_type = HOLEY;
      else if (strcmp(type_graph, "HR") == 0)
        g_type = 5;  // HOLEY_RANDOM
      else
        goto unknown;
      break;

    case 'S':
    case 's':
      if (strcmp(type_graph, "S") == 0)
        g_type = 6;  // SPAN
      else
        goto unknown;
      break;

    default:
    unknown:
      fprintf(stderr, "Type de graphe inconnu «%s», je tombe sur TRIANGULAR\n", type_graph);
      g_type = TRIANGULAR;
  }

  printf("Type de graphe: %s (enum=%d)\n", type_graph, g_type);
  printf("Taille de la maille: %d\n", size_mesh);
  first_step(argc, argv);

  struct move_t *first_move  = make_first_move();
  struct move_t *first_move2 = makee_first_move();

  if (syntax_test(argc) == -1) {
    return EXIT_FAILURE;
  }

  int start_player   = player_to_start();
  int current_player = start_player;
  int other_player   = (start_player + 1) % NUM_PLAYERS;

  struct graph_t *g1 = createGraph(size_mesh, g_type);
  struct graph_t *g2 = createGraph(size_mesh, g_type);

  struct graph_t *graphs[2] = {g1, g2};
  // struct graph_t *globalGraph = g1;
  struct board_t *board = board_init();
  board->graph          = g1;

  vertex_t last_positions[2];
  vertex_t current_positions[2];

  players[current_player].initialize(current_player, graphs[current_player]);
  players[other_player].initialize(other_player, graphs[other_player]);

  vertex_t first_positions[2] = {board->graph->start[0], board->graph->start[1]};

  players[current_player].pos_actuel = board->graph->start[current_player];
  players[other_player].pos_actuel   = board->graph->start[other_player];

  last_positions[current_player] = board->graph->start[current_player];
  last_positions[other_player]   = board->graph->start[other_player];

  current_positions[current_player] = board->graph->start[current_player];
  current_positions[other_player]   = board->graph->start[other_player];

  printf("First player: %s, at position %d\n", players[start_player].get_player_name(),
         players[start_player].pos_actuel);
  printf("Second player: %s, at position %d\n", players[other_player].get_player_name(),
         players[other_player].pos_actuel);

  int winner     = -1;
  int turn_count = 0;

  add_move_to_board(board, *first_move);
  printf("The size of the board is: %d vertices \n", board->graph->num_vertices);
  printf("----------Starting Game----------\n");

  struct move_t current_move = *first_move;
  printf("The server did the first move : %s\n", move_type_to_string(current_move.t));
  printf("In the vertex %d \n", current_move.m);
  if (affichage)
    print_hex_grid(board->graph);
  printf("The number of moves played so far is: %d\n", board->size_moves);
  struct move_t moves_act[2] = {current_move, current_move};

  while (winner == -1 && turn_count < max_turns) {
    struct player_tt *current_player_ptr = malloc(sizeof(struct player_tt));
    current_player_ptr->position         = players[current_player].pos_actuel;
    current_player_ptr->c                = players[current_player].player_color;
    current_player_ptr->last_position    = last_positions[current_player];

    // l'autre joueur
    struct player_tt *other_player_ptr = malloc(sizeof(struct player_tt));
    other_player_ptr->position         = players[other_player].pos_actuel;
    other_player_ptr->c                = players[other_player].player_color;
    other_player_ptr->last_position    = last_positions[other_player];

    struct move_t move = players[current_player].play(moves_act[other_player]);

    if (move.t == MOVE) {
      players[current_player].pos_actuel            = move.m;
      graphs[current_player]->start[current_player] = move.m;
      graphs[other_player]->start[current_player]   = move.m;
    }

    moves_act[current_player]                = move;
    last_positions[current_player]           = current_positions[current_player];
    current_positions[current_player]        = move.m;
    board->current_positions[current_player] = current_positions[current_player];

    if (move.t == MOVE &&
        !valid_move(board->graph, current_player_ptr, move.m, moves_act[other_player].m)) {
      int id = move.c;
      if (id != current_player) {
        printf("Player %s with id %d returned a move with id: %d\n",
               players[current_player].get_player_name(), current_player, move.c);
      }
      if (affichage)
        print_hex_grid(board->graph);
      printf("Player %s executed an illegal move of type RIP %s\n",
             players[current_player].get_player_name(), move_type_to_string(move.t));
      printf("from %d to %d ", last_positions[current_player], move.m);
      printf("Turn %d: Player %s plays %s from %u to vertex %u, his id is: %d\n", turn_count,
             players[current_player].get_player_name(), move_type_to_string(move.t),
             players[current_player].pos_actuel, move.m, current_player);
      last_positions[current_player] = players[current_player].pos_actuel;
      winner                         = (current_player + 1) % NUM_PLAYERS;
      free(current_player_ptr);
      free(other_player_ptr);
      break;
    }
    board->graph->start[current_player] = players[current_player].pos_actuel;
    if (affichage) {
      print_hex_grid(board->graph);
      print_walls(board->graph);
    }
    if (move.t == WALL) {
      printf("the player %s put a wall from %d to %d and from %d to %d ",
             players[current_player].get_player_name(), move.e[0].fr, move.e[0].to, move.e[1].fr,
             move.e[1].to);

      struct player_tt dummy = {.position      = players[current_player].pos_actuel,
                                .last_position = last_positions[current_player],
                                .walls         = 10,
                                .c             = current_player};
      // place_wall(board->graph, &dummy, move);
      place_wall(graphs[current_player], &dummy, move);
      place_wall(graphs[other_player], &dummy, move);
    }

    if (move.t != NO_TYPE) {
      if (move.t == MOVE && move.m == first_positions[current_player] &&
          (int)move.c == current_player && turn_count > 2) {
        printf("Player %s claims victory by returning to start after visiting all objectives!\n",
               players[current_player].get_player_name());
        winner = current_player;
        free(current_player_ptr);
        free(other_player_ptr);
        break;
      }
      add_move_to_board(board, move);
      current_move   = move;
      current_player = (current_player + 1) % NUM_PLAYERS;
      other_player   = (current_player + 1) % NUM_PLAYERS;
      turn_count++;
    } else {
      printf("Invalid move by player %s — they lose!\n", players[current_player].get_player_name());
      winner = (current_player + 1) % NUM_PLAYERS;
      free(current_player_ptr);
      free(other_player_ptr);
      break;
    }

    // print_hex_grid(board->graph);
    free(current_player_ptr);
    free(other_player_ptr);
  }

  graph_to_dot(board->graph, "graph.dot");

  if (winner >= 0) {
    printf("\nPlayer %s wins the game!\n", players[winner].get_player_name());
  } else {
    printf("\nGame ended in a draw (max turns reached)\n");
  }
  printf("----------The END----------\n");

  for (int i = 0; i < NUM_PLAYERS; i++) {
    players[i].finalize();
    dlclose(players[i].library);
  }
  free(board->moves);
  free(board);
  free(first_move);
  free(first_move2);

  return 0;
}
