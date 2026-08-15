#ifndef GAME_H
#define GAME_H

#include "types.h"

/*
runs turns and rounds until one player is left or 500 rounds pass
*/
void game_loop(struct Monopoly_S *monopoly);

/*
prints the opening banner
*/
void start_msg(struct Player_S players[4]);

/*
creates the four strategy players
*/
void init_players_all(struct Player_S players[4]);

/*
sets the starting game state and shuffles the decks
*/
void init_game_state(struct Monopoly_S *monopoly);

/*
one six sided die
*/
int roll_dice();

/*
rolls for turn order and returns the first player
*/
int first_player(struct Monopoly_S *monopoly);

/*
1 once every solvent player has passed GO
*/
int is_round_passed(struct Monopoly_S *monopoly);

/*
maintenance and repairs before the dice are rolled
*/
void before_roll(struct Monopoly_S *monopoly);

/*
resolves whatever square the current player landed on
*/
void player_land(struct Monopoly_S *monopoly);

void land_property(struct Monopoly_S *monopoly);

void land_railway(struct Monopoly_S *monopoly);

void land_event(struct Monopoly_S *monopoly);

void land_tax(struct Monopoly_S *monopoly);

void land_insurance(struct Monopoly_S *monopoly);

void land_special(struct Monopoly_S *monopoly);

void land_utility(struct Monopoly_S *monopoly);

void land_bank(struct Monopoly_S *monopoly);

/*
runs an auction and returns the winning bid
*/
int auction(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
1 if the player owns every property in a colour group
*/
int is_monopoly(struct Monopoly_S *monopoly, struct Player_S *player, enum PropertyType_E p_type);

/*
builds houses and hotels across everything the current player owns
*/
void develope_property(struct Monopoly_S *monopoly);

/*
eliminates the current player when net worth goes negative
*/
void check_bankrupt(struct Monopoly_S *monopoly);

/*
prints every player standing at the end of a round
*/
void round_summary(struct Monopoly_S *monopoly);

/*
prints the winner and their final position
*/
void end_game_summary(struct Monopoly_S *monopoly);

#endif
