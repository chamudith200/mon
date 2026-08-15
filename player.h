#ifndef PLAYER_H
#define PLAYER_H


void player_init(struct Player_S *player, int id, enum PlayerType_E type, const char *name);

void player_move(struct Monopoly_S *monopoly);

void player_buy_property(struct Monopoly_S *monopoly);

void player_property_pay_rent(struct Monopoly_S *monopoly);

void player_buy_util_railway(struct Monopoly_S *monopoly);

void player_railway_pay_rent(struct Monopoly_S *monopoly);

void player_util_pay_rent(struct Monopoly_S *monopoly);

void player_go_to_jail(struct Monopoly_S *monopoly);

void player_develop_house(struct Monopoly_S *monopoly, struct Square_S *sq);

void player_develop_hotel(struct Monopoly_S *monopoly, struct Square_S *sq);

int player_max_loan_amount(struct Monopoly_S *monopoly, struct Player_S *player);

void player_renovate_property(struct Monopoly_S *monopoly);

void player_pay_taxes(struct Monopoly_S *monopoly);

void  player_pay_community_fund(struct Monopoly_S *monopoly);

int attempt_raise_cash(struct Monopoly_S *monopoly, int required_min);

void eliminate_player(struct Monopoly_S *monopoly, struct Player_S *player);

#endif