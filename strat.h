#ifndef STRAT_H
#define STRAT_H

#include "types.h"

/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_propert_buy(struct Monopoly_S *monopoly);

/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_railway_buy(struct Monopoly_S *monopoly);

/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_util_buy(struct Monopoly_S *monopoly);

/*
0 - withdraw
1 - raise the bid
*/
int eval_auction_bid(struct Monopoly_S *monopoly, struct Player_S *player, struct Square_S *sq, int current_bid);

/*
0 - no
1 - house
2 - hotel
*/
int eval_develop_property(struct Monopoly_S *monopoly, struct Property_S *prop);

/*
lowest house count across a colour group
*/
int eval_min_house(struct Monopoly_S *monopoly, enum PropertyType_E p_type);

/*
0 - leave it
1 - renovate
*/
int eval_renovate_property(struct Monopoly_S *monopoly);

/*
0 - no loan
else - the amount to borrow
*/
int eval_loan_amount(struct Monopoly_S *monopoly, int max_loan);

/*
0 - nothing
1 - part repayment
2 - full repayment
3 - extend
4 - borrow more
*/
int eval_repay_loan(struct Monopoly_S *monopoly);

/*
0 - skip
1 - maintain
*/
int eval_maintain_building(struct Monopoly_S *monopoly, struct Property_S *prop);

/*
NULL if nothing worth insuring, otherwise the square and the policy to buy
*/
struct Square_S *eval_insurance_target(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_type);

#endif
