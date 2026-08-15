#include "strat.h"
#include "types.h"
#include "game.h"
#include "player.h"
#include <stdio.h>


/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_propert_buy(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *prop = &monopoly->board[player->current_pos].properties;
    
    if (prop->current_owner != PL_NONE) return 0;

    if (monopoly->game_state.active_gov_reg == GR_ANTI_SPECULATION_ACT) {
        int undeveloped = 0;
        for (int i = 0; i < 40; i++) {
            struct Square_S *sq = &monopoly->board[i];
            if (sq->type != ST_PROPERTY) continue;
            if (sq->properties.current_owner != player->id) continue;
            if (sq->properties.house_count == 0 && sq->properties.hotel_count == 0) undeveloped++;
        }
        if (undeveloped >= 3) return -1;
    }

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (player->cash >= prop->purchase_price + prop->base_rental) return 1;
            return -1;

        case PL_CONSERVATIVE_BANKER:
            if (player->cash - prop->purchase_price >= player->cash / 2) return 1;
            return -1;

        case PL_RISK_TAKER:
            if (player->cash >= prop->purchase_price) return 1;
            return -1;

        case PL_OPPORTUNISTIC_TRADER:
            if (player->cash >= prop->purchase_price && monopoly->game_state.market_boom_group == prop->type) return 1;
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

    if (player->cash < rail->purchase_price) return -1;
    if (rail->current_owner != PL_NONE) return 0;
    return 1;
}


/*
0  - don't buy
1  - buy
-1 - auction
*/
int eval_util_buy(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *util = &monopoly->board[player->current_pos].properties;

    if (player->cash < util->purchase_price) return -1;
    if (util->current_owner != PL_NONE) return 0;
    return 1;
}


int eval_auction_bid(struct Player_S *player, struct Property_S *prop, int current_bid) {
    if (player->cash < current_bid + 250) return 0;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (current_bid < prop->purchase_price * 1.2) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (current_bid < prop->purchase_price) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (player->cash >= current_bid + 250) return 1;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (current_bid < prop->purchase_price * 0.9) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0- no
1- house
2- hotel
*/
int eval_develop_property(struct Monopoly_S *monopoly, struct Property_S *prop) {    
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (is_monopoly(monopoly, player, prop->type) == 0) return 0;

    int min_house = eval_min_house(monopoly, prop->type);
    if (prop->house_count > min_house) return 0;
    if (prop->hotel_count == 1) return 0;

    switch(player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (player->cash >= prop->house_construction_cost && prop->house_count < 4) return 1;
            if (player->cash >= prop->hotel_construction_cost && min_house == 4 && prop->hotel_count == 0) return 2;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (player->cash >= prop->house_construction_cost && prop->house_count < 4) return 1;
            if (player->cash >= prop->hotel_construction_cost && min_house == 4 && player->has_active_loan == 0 && prop->hotel_count == 0) return 2;
            return 0;

        case PL_RISK_TAKER:
            if (player->cash >= prop->house_construction_cost && prop->house_count < 4) return 1;
            if (player->cash >= prop->hotel_construction_cost && min_house == 4 && prop->hotel_count == 0) return 2;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (player->cash >= prop->house_construction_cost && prop->house_count < 4) {
                if (monopoly->game_state.current_inflation_rate <= 0 || monopoly->game_state.active_gov_reg == GR_HOUSING_SUBSIDY) return 1;
            }
            if (player->cash >= prop->hotel_construction_cost && min_house == 4 && prop->hotel_count == 0) {
                if (monopoly->game_state.current_inflation_rate <= 0 || monopoly->game_state.active_gov_reg == GR_HOUSING_SUBSIDY) return 2;
            }
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


int eval_min_house(struct Monopoly_S *monopoly, enum PropertyType_E p_type) {
    int house_count[4];
    int house_count_i = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &monopoly->board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *prop = &sq->properties;
        if (prop->type != p_type) continue;

        house_count[house_count_i++] = prop->house_count;
    }

    int min = house_count[0];
    for (int i = 1; i < house_count_i; i++) {
        if (house_count[i] < min) min = house_count[i];
    }
    return min;
}


int eval_renovate_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *property = &monopoly->board[player->current_pos].properties;

    if (property->depreciation_precent == 0) return 0;
    if (property->property_age <= 50) return 0;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (property->depreciation_precent > 0) return 1;

        case PL_CONSERVATIVE_BANKER:
            if (property->depreciation_precent > 10) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (property->depreciation_precent == 30) return 1;
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
0 - dont
1-4 - house count
5- hotel
*/
int eval_maintaisn_buildings(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *property = &monopoly->board[player->current_pos].properties;    

    if (property->house_count == 0 && property->hotel_count == 0) return 0;
    
    return 0;
}

struct Property_S *eval_player_buy_insurance(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_tyte) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->property_owned == 0) return NULL;
    return NULL;
}


/*
0 - no loan
else - amount to borrow
*/
int eval_loan_amount(struct Monopoly_S *monopoly, int max_loan) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan) return 0;
    if (max_loan < 1000) return 0;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            return max_loan;

        case PL_CONSERVATIVE_BANKER:
            if (player->cash > 10000) return 0;
            return max_loan / 4;

        case PL_RISK_TAKER:
            return max_loan;

        case PL_OPPORTUNISTIC_TRADER:
            if (monopoly->game_state.current_interest_rate > 10) return 0;
            return max_loan / 2;

        case PL_NONE:
            return 0;
    }
    return 0;
}


/*
0 - don't repay
1 - part repayment
2 - full repayment
3 - extend
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
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (player->cash >= outstanding) return 2;
            if (player->loan_round_remaining <= 5) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


int eval_maintain_building(struct Monopoly_S *monopoly, struct Property_S *prop) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int condition = prop->hotel_count ? prop->hotel_condition : prop->house_condition[0];

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            if (condition < 90) return 1;
            return 0;

        case PL_CONSERVATIVE_BANKER:
            if (condition < 75) return 1;
            return 0;

        case PL_RISK_TAKER:
            if (condition < 50) return 1;
            return 0;

        case PL_OPPORTUNISTIC_TRADER:
            if (condition < 75 && player->cash > 5000) return 1;
            return 0;

        case PL_NONE:
            return 0;
    }
    return 0;
}


int eval_insurance_target(struct Monopoly_S *monopoly, enum InsuranceType_E *ins_type) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    *ins_type = INS_NONE;
    if (player->property_owned == 0) return -1;

    int best = -1;
    int best_val = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->insurance_type != INS_NONE) continue;
        if (property->house_count == 0 && property->hotel_count == 0) continue;

        int val = property->market_price;
        if (val > best_val) {
            best_val = val;
            best = i;
        }
    }

    if (best < 0) return -1;

    switch (player->type) {
        case PL_AGGRESIVE_INVESTOR:
            *ins_type = INS_COMPREHENSIVE;
            break;

        case PL_CONSERVATIVE_BANKER:
            *ins_type = INS_COMPREHENSIVE;
            break;

        case PL_RISK_TAKER:
            *ins_type = INS_BASIC;
            break;

        case PL_OPPORTUNISTIC_TRADER:
            if (board[best].properties.hotel_count) *ins_type = INS_BUSINESS_INTERRUPTION;
            else *ins_type = INS_BASIC;
            break;

        case PL_NONE:
            return -1;
    }

    return best;
}