#include "strat.h"
#include "types.h"
#include "game.h"
#include "player.h"
#include "events.h"
#include "finance.h"

#include <stdio.h>


/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_propert_buy(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Player_S *player = &monopoly->players[game_state->current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *prop = &sq->properties;

    if (prop->current_owner != PL_NONE) return 0;

    if (game_state->active_gov_reg == GR_ANTI_SPECULATION_ACT) {
        int undeveloped = 0;
        for (int i = 0; i < 40; i++) {
            struct Square_S *own = &monopoly->board[i];
            if (own->type != ST_PROPERTY) continue;
            if (own->properties.current_owner != player->id) continue;
            if (own->properties.house_count == 0 && own->properties.hotel_count == 0) undeveloped++;
        }
        if (undeveloped >= 3) return -1;
    }

    int price = prop->purchase_price;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (player->cash >= price + prop->current_rent) return 1;
            return -1;

        case PL_CONSERVATIVE_BANKER:
            if (game_state->active_economic_event == EE_ECONOMIC_RECESSION) return -1;
            if (player->cash - price >= player->cash / 2) return 1;
            return -1;

        case PL_RISK_TAKER:
            if (player->cash >= price) return 1;
            return -1;

        case PL_OPPORTUNISTIC_TRADER:
            if (player->cash < price) return -1;
            if (property_value(monopoly, sq) > price + prop->house_construction_cost) return 1;
            if (prop->type == game_state->market_boom_group) return 1;
            return -1;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_railway_buy(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *rail = &monopoly->board[player->current_pos].properties;

    if (rail->current_owner != PL_NONE) return 0;
    if (player->cash < rail->purchase_price) return -1;

    switch (player->type) {
        case PL_CONSERVATIVE_BANKER:
        case PL_OPPORTUNISTIC_TRADER:
            return 1;

        case PL_AGGRESIVE_INVESTOR:
        case PL_RISK_TAKER:
            if (player->cash >= rail->purchase_price * 2) return 1;
            return -1;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_util_buy(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *util = &monopoly->board[player->current_pos].properties;

    if (util->current_owner != PL_NONE) return 0;
    if (player->cash < util->purchase_price) return -1;

    switch (player->type) {
        case PL_CONSERVATIVE_BANKER:
        case PL_OPPORTUNISTIC_TRADER:
            return 1;

        case PL_AGGRESIVE_INVESTOR:
        case PL_RISK_TAKER:
            if (player->cash >= util->purchase_price * 2) return 1;
            return -1;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - withdraw
1 - raise the bid
*/
int eval_auction_bid(struct Monopoly_S *monopoly, struct Player_S *player, struct Square_S *sq, int current_bid) {
    int next_bid = current_bid + 250;

    if (player->cash < next_bid) return 0;

    int value = property_value(monopoly, sq);

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (next_bid < value * 120 / 100) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (next_bid < value) return 1;
            return 0;

        case PL_RISK_TAKER:
            return 1;

        case PL_OPPORTUNISTIC_TRADER:
            if (next_bid < value * 90 / 100) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - no
1 - house
2 - hotel
*/
int eval_develop_property(struct Monopoly_S *monopoly, struct Property_S *prop) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Player_S *player = &monopoly->players[game_state->current_player];

    if (is_monopoly(monopoly, player, prop->type) == 0) return 0;
    if (prop->hotel_count == 1) return 0;

    int min_house = eval_min_house(monopoly, prop->type);
    if (prop->house_count > min_house) return 0;

    int can_house = (player->cash >= prop->house_construction_cost && prop->house_count < 4);
    int can_hotel = (player->cash >= prop->hotel_construction_cost && min_house == 4);

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (can_hotel) return 2;
            if (can_house) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (can_hotel && player->has_active_loan == 0) return 2;
            if (can_house) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (can_hotel) return 2;
            if (can_house) return 1;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (game_state->current_inflation_rate > 0 && game_state->active_gov_reg != GR_HOUSING_SUBSIDY) return 0;
            if (can_hotel) return 2;
            if (can_house) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
lowest house count across a colour group
*/
int eval_min_house(struct Monopoly_S *monopoly, enum PropertyType_E p_type) {
    int min = 4;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &monopoly->board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *prop = &sq->properties;
        if (prop->type != p_type) continue;

        int count = prop->hotel_count ? 4 : prop->house_count;
        if (count < min) min = count;
    }
    return min;
}


/*
0 - leave it
1 - renovate
*/
int eval_renovate_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *property = &monopoly->board[player->current_pos].properties;

    if (property->is_damaged) return 1;
    if (property->depreciation_precent == 0) return 0;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (property->depreciation_precent > 5) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (property->depreciation_precent > 10) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (property->depreciation_precent >= 30) return 1;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (property->depreciation_precent > 15) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - no loan
else - the amount to borrow
*/
int eval_loan_amount(struct Monopoly_S *monopoly, int max_loan) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Player_S *player = &monopoly->players[game_state->current_player];

    if (player->has_active_loan) return 0;
    if (max_loan < 1000) return 0;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            return max_loan;

        case PL_CONSERVATIVE_BANKER:
            if (player->cash > 5000) return 0;
            return max_loan / 4;

        case PL_RISK_TAKER:
            return max_loan;

        case PL_OPPORTUNISTIC_TRADER:
            if (game_state->current_interest_rate > 10) return 0;
            return max_loan / 2;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - nothing
1 - part repayment
2 - full repayment
3 - extend
4 - borrow more
*/
int eval_repay_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan == 0) return 0;

    int outstanding = player->loan_amount + player->accrued_interest;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (player->cash >= outstanding * 2) return 2;
            if (player->loan_round_remaining <= 3) return 3;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (player->cash >= outstanding) return 2;
            if (player->cash >= outstanding / 4) return 1;
            return 3;

        case PL_RISK_TAKER:
            if (player->loan_round_remaining <= 2) return 3;
            return 4;

        case PL_OPPORTUNISTIC_TRADER:
            if (player->cash >= outstanding) return 2;
            if (player->loan_round_remaining <= 5) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - skip
1 - maintain
*/
int eval_maintain_building(struct Monopoly_S *monopoly, struct Property_S *prop) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int condition = prop->hotel_count ? prop->hotel_condition : prop->house_condition[0];

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (condition < 90) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (condition < 90) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (condition < 25) return 1;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (condition < 75 && player->cash > 5000) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
NULL if nothing worth insuring, otherwise the square and the policy to buy
*/
struct Square_S *eval_insurance_target(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_type) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    *ins_type = INS_NONE;
    if (player->property_owned == 0) return NULL;
    if (player->type == PL_RISK_TAKER && player->suffered_loss == 0) return NULL;

    struct Square_S *best = NULL;
    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->insurance_type != INS_NONE) continue;
        if (property->house_count == 0 && property->hotel_count == 0) continue;

        if (best == NULL || property_value(monopoly, sq) > property_value(monopoly, best)) best = sq;
    }

    if (best == NULL) return NULL;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (best->properties.hotel_count) *ins_type = INS_COMPREHENSIVE;
            else *ins_type = INS_BASIC;
            break;

        case PL_CONSERVATIVE_BANKER:
            *ins_type = INS_COMPREHENSIVE;
            break;

        case PL_RISK_TAKER:
            *ins_type = INS_BASIC;
            break;

        case PL_OPPORTUNISTIC_TRADER:
            if (best->properties.hotel_count) *ins_type = INS_BUSINESS_INTERRUPTION;
            else return NULL;
            break;

        case PL_NONE:
            return NULL;
    }

    return best;
}
