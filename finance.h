#ifndef FINANCE_H
#define FINANCE_H

#include "types.h"

int economy_interest_rate(struct Monopoly_S *monopoly);

int player_collateral_value(struct Monopoly_S *monopoly, struct Player_S *player);


void player_take_loan(struct Monopoly_S *monopoly);

void player_repay_loan(struct Monopoly_S *monopoly, int amount);

void player_extend_loan(struct Monopoly_S *monopoly);

void accrue_loan_interest(struct Monopoly_S *monopoly);

void loan_default(struct Monopoly_S *monopoly, struct Player_S *player);


int insurance_premium(struct Monopoly_S *monopoly, int pos, enum InsuranceType_E type);

void player_buy_insurance(struct Monopoly_S *monopoly);

void insurance_tick(struct Monopoly_S *monopoly);

int insurance_payout(struct Monopoly_S *monopoly, int pos, int repair_cost);


void random_disaster(struct Monopoly_S *monopoly);

void repair_property(struct Monopoly_S *monopoly, int pos);

void maintain_buildings(struct Monopoly_S *monopoly);

const char *disaster_name(int disaster);

#endif
