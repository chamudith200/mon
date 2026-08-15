#ifndef STRAT_H
#define STRAT_H

#include "types.h"

int eval_propert_buy(struct Monopoly_S *monopoly);

int eval_railway_buy(struct Monopoly_S *monopoly);

int eval_util_buy(struct Monopoly_S *monopoly);

int eval_auction_bid(struct Player_S *player, struct Property_S *prop, int current_bid);

int eval_develop_property(struct Monopoly_S *monopoly, struct Property_S *prop);

int eval_min_house(struct Monopoly_S *monopoly, enum PropertyType_E p_type);

int eval_renovate_property(struct Monopoly_S *monopoly);

int eval_maintaisn_buildings(struct Monopoly_S *monopoly);

int eval_loan_amount(struct Monopoly_S *monopoly, int max_loan);

int eval_repay_loan(struct Monopoly_S *monopoly);

int eval_maintain_building(struct Monopoly_S *monopoly, struct Property_S *prop);

int eval_insurance_target(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_type);

struct Property_S *eval_player_buy_insurance(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_tyte);


#endif