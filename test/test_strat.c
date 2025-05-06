

/*void test_evaluation_functions() {
    printf("=== Test des fonctions d'évaluation ===\n");

    // Initialisation du graphe
    int m = 5;
    struct graph_t *graph = createGraph(m, TRIANGULAR);

    // Configuration des objectifs
    graph->num_objectives = 1;
    graph->objectives = malloc(3 * sizeof(vertex_t));
    graph->objectives[0] = axial_to_index(2, 0, m); // Objectif nord-ouest
    printf("Objectif défini à l'index : %d\n", graph->objectives[0]);

    // Initialisation de l'état du jeu
    struct game_state state;
    state.graph = graph;

    // Position des joueurs
    state.previous_moves[0] =
        (struct move_t){.m = axial_to_index(2, -1, m), .t = MOVE, .c = 0};
    state.previous_moves[1] =
        (struct move_t){.m = axial_to_index(1, -1, m), .t = MOVE, .c = 1};
    state.previous_positions[0] = axial_to_index(1, -1, m);
    state.previous_positions[1] = axial_to_index(0, -1, m);
    state.graph->start[0] = state.previous_moves[0].m;
    state.graph->start[1] = state.previous_moves[1].m;
    print_hex_grid(state.graph);

    // === Test de la fonction available moves  ===
    printf("\n--- Test des déplacements disponibles ---\n");
    struct player_tt *player = malloc(sizeof(struct player_tt));
    player->position = state.previous_moves[0].m;
    player->last_position = state.previous_positions[0];
    player->c = 0;
    struct move_t legal_moves[1069];
    int nuum_moves =
        availableMoves(legal_moves, graph, player, state.previous_moves[1].m);
    printf("Position du joueur 0 : %d\n", player->position);
    printf("Nombre de déplacements possibles : %d\n", nuum_moves);
    for (int i = 0; i < nuum_moves; i++) {
      printf("%u ", legal_moves[i].m);
    }
    printf("\n");

    // === Test de la fonction de chemin le plus court ===
    printf("\n--- Test du chemin le plus court ---\n");
    vertex_t *path = malloc(graph->num_vertices * sizeof(vertex_t));
    graph->objectives[0] = 3;
    graph->start[0] = state.previous_moves[0].m;
    graph->start[1] = 8;
    state.previous_moves[1].m = 8;
    state.previous_positions[1] = 7;
    printf("previous position  : %d\n", state.previous_positions[0]);
    printf("previous position  : %d\n", state.previous_positions[1]);
    printf("previous move  : %d\n", state.previous_moves[0].m);
    printf("previous move  : %d\n", state.previous_moves[1].m);
    print_hex_grid(graph);

    int length = shortest_path_length(
        graph, state.previous_moves[0].m, graph->objectives[0],
        state.previous_moves[1].m, path, 0); // 0 just for the current error
    printf(" Distance obtenue : %d\n", length);
    // print path
    for (int i = 0; path[i] != (unsigned int)-1; ++i) {
      printf("%d, ", path[i]);
    }
    printf("\n");

    // pareil pour l'autre joueur (path )
    vertex_t *path2 = malloc(graph->num_vertices * sizeof(vertex_t));
    graph->objectives[0] = 26;
    graph->start[0] = state.previous_moves[1].m;
    graph->start[1] = 0;
    state.previous_moves[0].m = 0;
    state.previous_positions[0] = 1;
    printf("previous position  : %d\n", state.previous_positions[0]);
    printf("previous position  : %d\n", state.previous_positions[1]);
    printf("previous move  : %d\n", state.previous_moves[0].m);
    printf("previous move  : %d\n", state.previous_moves[1].m);
    print_hex_grid(graph);
    length = shortest_path_length(graph, state.previous_moves[1].m,
                                  graph->objectives[0], state.previous_moves[0].m,
                                  path2, 0); // 0 just for the current error
    printf(" Distance obtenue : %d\n", length);
    // print path
    for (int i = 0; path2[i] != (unsigned int)-1; ++i) {
      printf("%d, ", path2[i]);
    }
    printf("\n");

    // assert(length == 2); // Le chemin le plus court est de longueur 2

    // === Test de la fonction d'évaluation ===
    printf("\n--- Évaluation ---\n");
    int eval_p0 = evaluate(&state, 0);
    int eval_p1 = evaluate(&state, 1);
    printf("Évaluation joueur 0 : %d\n", eval_p0);
    printf("Évaluation joueur 1 : %d\n", eval_p1);

    // Affichage des déplacements possibles
    struct move_t legafl_moves[1069];
    int num_moves =
        availableMoves(legafl_moves, graph, player, state.previous_moves[1].m);
    printf("Position du joueur 0 : %d\n", player->position);
    printf("Nombre de déplacements possibles : %d\n", num_moves);
    for (int i = 0; i < num_moves; i++) {
      printf("%u ", legafl_moves[i].m);
    }
    printf("\n");

    struct scored_move result_negamax = negamax_ab(&state, 5, -1, 1, 1);
    struct scored_move result_naive = negamax_naive(&state, 4, 1);
    struct scored_move result_minmax = minMax(&state, 1, 1);
    struct scored_move result_minmaxi = minMaxi(&state, 3, 1, 1);

    printf("\nRésultats :\n");
    printf("Negamax Alpha-Beta : Type = %s, Dest = %d, Score = %d\n",
           result_negamax.move.t == MOVE ? "MOVE" : "WALL", result_negamax.move.m,
           result_negamax.score);
    printf("Negamax Naïf      : Type = %s, Dest = %d, Score = %d\n",
           result_naive.move.t == MOVE ? "MOVE" : "WALL", result_naive.move.m,
           result_naive.score);
    printf("MinMax            : Type = %s, Dest = %d, Score = %d\n",
           result_minmax.move.t == MOVE ? "MOVE" : "WALL", result_minmax.move.m,
           result_minmax.score);
    printf("MinMaxi           : Type = %s, Dest = %d, Score = %d\n",
           result_minmaxi.move.t == MOVE ? "MOVE" : "WALL", result_minmaxi.move.m,
           result_minmaxi.score);

    // === Simulation de jeu (exemple avec MinMaxi jusqu'à l'arrivée à l'objectif)
    // ===
    printf("\n--- Simulation jusqu'à objectif pour joueur 0 ---\n");

    // Réinitialisation de la position
    state.previous_moves[0].m = 1;
    state.previous_moves[0].t = MOVE;
    state.previous_moves[0].c = 0;
    state.previous_moves[1].m = axial_to_index(1, -1, m);
    state.previous_moves[1].t = MOVE;
    state.previous_moves[1].c = 1;
    state.previous_positions[0] = 0;
    state.previous_positions[1] = axial_to_index(0, -1, m);

    // Vérification du dernier coup
    int valid = (result_minmaxi.move.t == MOVE)
                    ? valid_move(graph, player, result_minmaxi.move.m,
                                 state.previous_moves[1].m)
                    : valid_wall(graph, player, result_minmaxi.move);

    printf("Coup %s valide : %s\n",
           result_minmaxi.move.t == MOVE ? "MOVE" : "WALL",
           valid ? "OUI" : "NON");

    // === Nettoyage ===
    // free(graph->objectives);
    // graph_free(graph);
    // free(player);

    printf("=== Tests terminés ===\n");
  }

  // Fonction pour initialiser un joueur
  void init_player(struct player_tt *p, vertex_t pos, vertex_t last_pos,
                   enum player_color_t color, int walls) {
    p->position = pos;
    p->last_position = last_pos;
    p->c = color;
    p->walls = walls;
  }

  // Test 1 : Chemin direct sans mur
  void test_shortest_path_no_wall() {
    printf("=== Test 1 : Chemin direct sans mur ===\n");
    int m = 15;
    struct graph_t *g = createGraph(m, HOLEY);
    g->objectives[0] = 610;
    print_hex_grid(g);
    vertex_t start = 0; // Position initiale (0,0)
    vertex_t objective = g->objectives[0];
    vertex_t *path = malloc(g->num_vertices * sizeof(vertex_t));
    int length = shortest_path_length(g, start, objective, 10, path, 0);
    printf("Distance attendue : 2 | Distance obtenue : %d\n", length);
    // assert(length == 2); // Le chemin le plus court est de longueur 2
    for (vertex_t v = 0; path[v] != (unsigned int)-1; ++v) {
      printf("%d, ", path[v]);
    }
    printf("\n");
    free(path);

    graph_free(g);
    printf("✔️ Test réussi.\n\n");
  }

*/