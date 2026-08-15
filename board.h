#ifndef BOARD_H
#define BOARD_H

#include "types.h"

/*
sets the id, name and type of a single square
*/
void board_init(struct Square_S *square, int id, const char *name, enum SquareType_E type);

/*
names and types every square on the board
*/
void inti_squares_all(struct Square_S board[40]);

/*
sets prices, rents and build costs for every property group
*/
void init_propert_all(struct Square_S board[40]);

/*
sets ownership and price for railways and utilities
*/
void init_railways_utils(struct Square_S board[40]);

/*
rolls two dice and advances the current player, handles jail and GO
*/
void player_move(struct Monopoly_S *monopoly);

#endif
