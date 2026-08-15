#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

/*
builds and shuffles both card decks before the game starts
*/
void init_decks(struct Monopoly_S *monopoly);

/*
fills a deck with card indexes 0 to count - 1
*/
void deck_init(struct Deck_S *deck, int count);

/*
fisher yates shuffle over the whole deck
*/
void deck_shuffle(struct Deck_S *deck);

/*
takes the top card and sends it to the back of the deck
*/
int deck_draw(struct Deck_S *deck);


/*
sets every economic event multiplier back to 100
*/
void reset_ee_mult(struct Monopoly_S *monopoly);

/*
sets every national event multiplier back to 100
*/
void reset_ne_mult(struct Monopoly_S *monopoly);

/*
sets every regional development multiplier back to 100
*/
void reset_rd_mult(struct Monopoly_S *monopoly);


/*
picks a random economic event every 15 rounds
*/
void draw_economic_event(struct Monopoly_S *monopoly);

/*
draws the top national event card for the current player
*/
void draw_national_event(struct Monopoly_S *monopoly);

/*
draws the top regional development card every 15 rounds
*/
void draw_regional_card(struct Monopoly_S *monopoly);

/*
picks a random government regulation every 20 rounds
*/
void draw_gov_regulation(struct Monopoly_S *monopoly);

/*
picks a boom group and a decline group, both on a 30 round cooldown
*/
void market_review(struct Monopoly_S *monopoly);

/*
counts down every active effect and clears the expired ones
*/
void expire_events(struct Monopoly_S *monopoly);

/*
prints the active market conditions at the end of a round
*/
void market_conditions(struct Monopoly_S *monopoly);


/*
market price of a square after every active event multiplier
*/
int property_value(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
percentage applied to residential rent on a square
*/
int event_rent_mult(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
percentage applied to hotel rent for an owner
*/
int event_hotel_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner);

/*
percentage applied to railway rent for an owner
*/
int event_rail_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner);

/*
percentage applied to utility rent for an owner
*/
int event_util_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner);

/*
percentage applied to house construction cost
*/
int event_house_cost_mult(struct Monopoly_S *monopoly, struct Player_S *player);

/*
percentage applied to hotel construction cost
*/
int event_hotel_cost_mult(struct Monopoly_S *monopoly, struct Player_S *player);

/*
percentage applied to an insurance premium
*/
int event_insurance_mult(struct Monopoly_S *monopoly);


/*
1 if the square sits on the coast
*/
int is_coastal(struct Square_S *sq);

/*
1 if the square is a southern coastal property
*/
int is_south_coastal(struct Square_S *sq);

/*
1 if the square is a commercial property
*/
int is_commercial(struct Square_S *sq);

/*
1 if the square is in the IT zone
*/
int is_it_zone(struct Square_S *sq);

/*
1 if the square is a northern property
*/
int is_north(struct Square_S *sq);

/*
1 if the square is in the airport zone
*/
int is_airport_zone(struct Square_S *sq);

/*
1 if the square is in the Kandy university zone
*/
int is_kandy_zone(struct Square_S *sq);

/*
1 if the square is in the Colombo port zone
*/
int is_colombo_zone(struct Square_S *sq);


/*
NULL if none, otherwise a random owned property
*/
struct Square_S *random_owned_property(struct Monopoly_S *monopoly, int developed_only);

/*
NULL if none, otherwise a random owned coastal property
*/
struct Square_S *random_coastal_property(struct Monopoly_S *monopoly);

/*
wrecks the buildings on a square and drops its market price
*/
void damage_property(struct Monopoly_S *monopoly, struct Square_S *sq);


const char *economic_event_name(int event);

const char *national_event_name(int card);

const char *regional_card_name(int card);

const char *gov_regulation_name(int reg);

const char *group_name(int group);

#endif
