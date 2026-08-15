#ifndef FINANCE_H
#define FINANCE_H

#include "types.h"

/*
recalculates the current player net worth
*/
void calc_networth(struct Monopoly_S *monopoly);

/*
rolls a new inflation rate every 10 rounds and compounds it into every price
*/
void apply_inflation(struct Monopoly_S *monopoly);

/*
ages every property, wears down buildings and applies structural damage
*/
void prop_building_condition(struct Monopoly_S *monopoly);

/*
clears depreciation and age on the square the current player stands on
*/
void player_renovate_property(struct Monopoly_S *monopoly);


/*
current bank interest rate for the prevailing economy
*/
int economy_interest_rate(struct Monopoly_S *monopoly);

/*
total mortgage value of everything a player can still pledge
*/
int player_max_loan_amount(struct Monopoly_S *monopoly, struct Player_S *player);

/*
takes a secured loan and locks the pledged collateral
*/
void player_take_loan(struct Monopoly_S *monopoly);

/*
pays down interest first then the principal
*/
void player_repay_loan(struct Monopoly_S *monopoly, int amount);

/*
adds 10 more rounds to the loan term
*/
void player_extend_loan(struct Monopoly_S *monopoly);

/*
borrows more against any collateral still free
*/
void player_increase_loan(struct Monopoly_S *monopoly);

/*
compounds interest on every loan and defaults the expired ones
*/
void accrue_loan_interest(struct Monopoly_S *monopoly);

/*
forecloses all pledged collateral and clears the debt
*/
void loan_default(struct Monopoly_S *monopoly, struct Player_S *player);


/*
premium for a policy on a square
*/
int insurance_premium(struct Monopoly_S *monopoly, struct Square_S *sq, enum InsuranceType_E type);

/*
buys a policy for one property when the player lands on an insurer
*/
void player_buy_insurance(struct Monopoly_S *monopoly);

/*
counts down every policy and warns 3 rounds before expiry
*/
void insurance_tick(struct Monopoly_S *monopoly);

/*
compensation a policy pays out against a repair bill
*/
int insurance_payout(struct Monopoly_S *monopoly, struct Square_S *sq, int repair_cost);

/*
1 if the policy on a square covers the given disaster
*/
int insurance_covers(struct Square_S *sq, int disaster);


/*
hits one random developed property every 10 rounds
*/
void random_disaster(struct Monopoly_S *monopoly);

/*
restores a damaged property once the owner can pay
*/
void repair_property(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
restores building condition at the start of a turn
*/
void maintain_buildings(struct Monopoly_S *monopoly);


/*
charges income tax on the current player
*/
void player_pay_taxes(struct Monopoly_S *monopoly);

/*
charges the community development fund levy
*/
void player_pay_community_fund(struct Monopoly_S *monopoly);

/*
charges hotel owners the luxury tax while that regulation is active
*/
void luxury_property_tax(struct Monopoly_S *monopoly);


/*
mortgages a square to the bank for its mortgage value
*/
int player_mortgage_property(struct Monopoly_S *monopoly, struct Square_S *sq);

/*
0 if impossible, otherwise the amount raised by mortgaging or selling
*/
int attempt_raise_cash(struct Monopoly_S *monopoly, int required_min);

const char *disaster_name(int disaster);

#endif
