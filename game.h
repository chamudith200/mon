#ifndef GAME_H
#define GAME_H

#include "types.h"


void game_loop(struct Monopoly_S *monopoly);


void start_msg(struct Player_S players[4]);


void init_players_all(struct Player_S players[4]);


void init_game_state(struct Monopoly_S *monopoly);


int roll_dice();


int first_player(struct Monopoly_S *monopoly);


int is_round_passed(struct Monopoly_S *monopoly);


void before_roll(struct Monopoly_S *monopoly);


void player_land(struct Monopoly_S *monopoly);

void land_property(struct Monopoly_S *monopoly);

void land_railway(struct Monopoly_S *monopoly);

void land_event(struct Monopoly_S *monopoly);

void land_tax(struct Monopoly_S *monopoly);

void land_insurance(struct Monopoly_S *monopoly);

void land_special(struct Monopoly_S *monopoly);

void land_utility(struct Monopoly_S *monopoly);

void land_bank(struct Monopoly_S *monopoly);


int auction(struct Monopoly_S *monopoly, struct Square_S *sq);


int is_monopoly(struct Monopoly_S *monopoly, struct Player_S *player, enum PropertyType_E p_type);


void develope_property(struct Monopoly_S *monopoly);


void check_bankrupt(struct Monopoly_S *monopoly);


void round_summary(struct Monopoly_S *monopoly);


void end_game_summary(struct Monopoly_S *monopoly);

#endif
