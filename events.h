#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

void init_decks(struct Monopoly_S *monopoly);

void deck_init(struct Deck_S *deck, int count);

void deck_shuffle(struct Deck_S *deck);

int deck_draw(struct Deck_S *deck);


void reset_ne_mult(struct Monopoly_S *monopoly);

void reset_rd_mult(struct Monopoly_S *monopoly);


void draw_national_event(struct Monopoly_S *monopoly);

void draw_regional_card(struct Monopoly_S *monopoly);

void draw_gov_regulation(struct Monopoly_S *monopoly);

void market_review(struct Monopoly_S *monopoly);

const char *gov_regulation_name(int reg);

const char *group_name(int group);

void expire_cards(struct Monopoly_S *monopoly);

void market_conditions(struct Monopoly_S *monopoly);


int is_coastal(int pos);

int is_south_coastal(int pos);

int is_commercial(int pos);

int is_it_zone(int pos);

int is_north(int pos);

int is_airport_zone(int pos);

int is_kandy_zone(int pos);

int is_colombo_zone(int pos);


int random_owned_property(struct Monopoly_S *monopoly, int developed_only);

int random_coastal_property(struct Monopoly_S *monopoly);

void damage_property(struct Monopoly_S *monopoly, int pos);

int property_market_value(struct Monopoly_S *monopoly, int pos);

int property_rent_mult(struct Monopoly_S *monopoly, int pos);

const char *national_event_name(int card);

const char *regional_card_name(int card);

#endif
