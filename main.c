#include "types.h"
#include "board.h"
#include "player.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int main() {
    // srand(1);
    srand(1);

    struct Monopoly_S monopoly;

    init_players_all(monopoly.players);
    inti_squares_all(monopoly.board);
    init_propert_all(monopoly.board);
    init_railways_utils(monopoly.board);
    init_game_state(&monopoly);

    start_msg(monopoly.players);

    game_loop(&monopoly);

    return 0;
}
