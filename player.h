#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

/*
sets a player back to the starting state
*/
void player_init(struct Player_S *player, int id, enum PlayerType_E type, const char *name);

/*
buys the square the current player stands on
*/
void player_buy_property(struct Monopoly_S *monopoly);

/*
pays property rent to the owner of the square landed on
*/
void player_property_pay_rent(struct Monopoly_S *monopoly);

/*
buys the railway or utility the current player stands on
*/
void player_buy_util_railway(struct Monopoly_S *monopoly);

/*
pays railway rent based on how many stations the owner holds
*/
void player_railway_pay_rent(struct Monopoly_S *monopoly);

/*
pays utility rent based on the dice value
*/
void player_util_pay_rent(struct Monopoly_S *monopoly);

/*
sends the current player to jail for 3 turns
*/
void player_go_to_jail(struct Monopoly_S *monopoly);

/*
builds one house on a square
*/
void player_develop_house(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
replaces four houses with a hotel
*/
void player_develop_hotel(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
declares a player bankrupt and returns every asset to the bank
*/
void eliminate_player(struct Monopoly_S *monopoly, struct Player_S *player);

#endif
