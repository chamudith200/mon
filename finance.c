#include "finance.h"
#include "types.h"
#include "game.h"
#include "player.h"
#include "strat.h"
#include "events.h"

#include <stdio.h>
#include <stdlib.h>


int economy_interest_rate(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    int rate = 8;
    int inflation = game_state->current_inflation_rate;

    if (inflation < 0) rate = 5;
    else if (inflation <= 2) rate = 8;
    else if (inflation <= 5) rate = 10;
    else if (inflation <= 8) rate = 12;
    else rate = 15;

    if (game_state->active_economic_event == EE_ECONOMIC_RECESSION) rate = 15;

    rate += game_state->ne_mult.loan_interest_change;
    if (game_state->active_gov_reg == GR_REDUCE_LOAN_INTEREST) rate -= 2;
    if (rate < 1) rate = 1;

    return rate;
}


int player_collateral_value(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct Square_S *board = monopoly->board;

    int mortgage_val = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        if (sq->properties.current_owner != player->id) continue;
        if (sq->properties.is_mortaged) continue;
        if (sq->properties.is_loan_locked) continue;
        mortgage_val += sq->properties.mortgage_val;
    }

    return mortgage_val;
}


void player_take_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    int max_loan = player_collateral_value(monopoly, player) * 75 / 100;
    if (max_loan <= 0) return;

    int amount = eval_loan_amount(monopoly, max_loan);
    if (amount <= 0) return;

    player->has_active_loan = 1;
    player->loan_amount = amount;
    player->accrued_interest = 0;
    player->loan_interest_rate = economy_interest_rate(monopoly);
    player->loan_round_remaining = 20;
    player->cash += amount;

    printf("%s obtained a secured loan.\n", player->name);
    printf("Loan Amount : LKR %d.\n", amount);
    printf("Collateral :\n");

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        if (sq->properties.current_owner != player->id) continue;
        if (sq->properties.is_mortaged) continue;
        sq->properties.is_loan_locked = 1;
        printf("%s\n", sq->name);
    }

    printf("Interest Rate : %d%%\n", player->loan_interest_rate);
    printf("Duration : %d Rounds\n", player->loan_round_remaining);
}


void player_repay_loan(struct Monopoly_S *monopoly, int amount) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    if (player->has_active_loan == 0) return;
    if (amount > player->cash) amount = player->cash;
    if (amount <= 0) return;

    int outstanding = player->loan_amount + player->accrued_interest;
    if (amount > outstanding) amount = outstanding;

    player->cash -= amount;

    if (amount >= player->accrued_interest) {
        amount -= player->accrued_interest;
        player->accrued_interest = 0;
        player->loan_amount -= amount;
    }
    else {
        player->accrued_interest -= amount;
    }

    printf("%s repaid LKR %d.\n", player->name, amount);

    if (player->loan_amount + player->accrued_interest <= 0) {
        player->has_active_loan = 0;
        player->loan_amount = 0;
        player->accrued_interest = 0;
        player->loan_round_remaining = 0;

        for (int i = 0; i < 40; i++) {
            if (board[i].properties.current_owner == player->id) board[i].properties.is_loan_locked = 0;
        }
        printf("Outstanding Balance :\nLKR 0.\n");
        return;
    }

    printf("Outstanding Balance :\nLKR %d.\n", player->loan_amount + player->accrued_interest);
}


void player_extend_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan == 0) return;

    player->loan_round_remaining += 10;
    printf("%s extended the loan period.\n", player->name);
    printf("Duration : %d Rounds\n", player->loan_round_remaining);
}


void accrue_loan_interest(struct Monopoly_S *monopoly) {
    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[i];
        if (player->is_bankrupt) continue;
        if (player->has_active_loan == 0) continue;

        int outstanding = player->loan_amount + player->accrued_interest;
        player->accrued_interest += outstanding * player->loan_interest_rate / 100;

        player->loan_round_remaining--;
        if (player->loan_round_remaining <= 0) {
            loan_default(monopoly, player);
        }
    }
}


void loan_default(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct Square_S *board = monopoly->board;

    int outstanding = player->loan_amount + player->accrued_interest;

    if (player->cash >= outstanding) {
        player->cash -= outstanding;
        player->has_active_loan = 0;
        player->loan_amount = 0;
        player->accrued_interest = 0;
        player->loan_round_remaining = 0;
        for (int i = 0; i < 40; i++) {
            if (board[i].properties.current_owner == player->id) board[i].properties.is_loan_locked = 0;
        }

        printf("%s repaid LKR %d.\n", player->name, outstanding);
        printf("Outstanding Balance :\nLKR 0.\n");
        return;
    }

    printf("%s has defaulted.\n", player->name);
    printf("Collateral has been foreclosed.\n");

    int foreclosed = 0;
    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        struct Property_S *property = &sq->properties;
        if (property->is_loan_locked == 0) continue;
        if (property->current_owner != player->id) continue;

        if (property->hotel_count && player->hotels_owned > 0) player->hotels_owned--;
        property->house_count = 0;
        property->hotel_count = 0;
        for (int j = 0; j < 4; j++) property->house_condition[j] = 100;
        property->hotel_condition = 100;
        property->insurance_type = INS_NONE;
        property->insuarance_rounds = 0;
        property->is_loan_locked = 0;
        property->is_damaged = 0;
        property->current_owner = PL_NONE;

        switch (sq->type) {
            case ST_PROPERTY: if (player->property_owned > 0) player->property_owned--; break;
            case ST_RAILWAY:  if (player->railway_owned > 0)  player->railway_owned--;  break;
            case ST_UTILITY:  if (player->util_owned > 0)     player->util_owned--;     break;
            default: break;
        }
        foreclosed++;
    }

    player->has_active_loan = 0;
    player->loan_amount = 0;
    player->accrued_interest = 0;
    player->loan_round_remaining = 0;

    printf("Outstanding debt cleared.\n");

    if (foreclosed && player->property_owned == 0 && player->railway_owned == 0 && player->util_owned == 0 && player->cash <= 0) {
        eliminate_player(monopoly, player);
    }
}


int insurance_premium(struct Monopoly_S *monopoly, int pos, enum InsuranceType_E type) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Property_S *property = &monopoly->board[pos].properties;

    int value = property_market_value(monopoly, pos);
    value += property->house_count * property->house_construction_cost;
    value += property->hotel_count * property->hotel_construction_cost;

    int premium = 0;
    switch (type) {
        case INS_BASIC:                 premium = value *  5 / 100; break;
        case INS_COMPREHENSIVE:         premium = value * 10 / 100; break;
        case INS_BUSINESS_INTERRUPTION: premium = value * 15 / 100; break;
        default: return 0;
    }

    premium = premium * game_state->ee_mult.insurance_premium_mult / 100;
    premium = premium * game_state->ne_mult.insurance_premium_mult / 100;
    if (game_state->active_gov_reg == GR_INSURANCE_REGULATION) premium = premium * 85 / 100;

    return premium;
}


void player_buy_insurance(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    enum InsuranceType_E type = INS_NONE;
    int pos = eval_insurance_target(monopoly, &type);
    if (pos < 0 || type == INS_NONE) return;

    int premium = insurance_premium(monopoly, pos, type);
    if (premium > player->cash) return;

    player->cash -= premium;
    board[pos].properties.insurance_type = type;
    board[pos].properties.insuarance_rounds = 20;

    switch (type) {
        case INS_BASIC:                 printf("Basic Property Insurance purchased.\n");      break;
        case INS_COMPREHENSIVE:         printf("Comprehensive Insurance purchased.\n");       break;
        case INS_BUSINESS_INTERRUPTION: printf("Business Interruption Insurance purchased.\n"); break;
        default: break;
    }
    printf("Property : %s\n", board[pos].name);
    printf("Premium : LKR %d.\n", premium);
}


void insurance_tick(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        struct Property_S *property = &board[i].properties;
        if (property->insurance_type == INS_NONE) continue;

        property->insuarance_rounds--;

        if (property->insuarance_rounds == 3) {
            printf("Insurance policy on %s expires in 3 rounds.\n", board[i].name);
        }

        if (property->insuarance_rounds <= 0) {
            property->insurance_type = INS_NONE;
            property->insuarance_rounds = 0;
            printf("Insurance policy on %s has expired.\n", board[i].name);
        }
    }
}


int insurance_payout(struct Monopoly_S *monopoly, int pos, int repair_cost) {
    struct Property_S *property = &monopoly->board[pos].properties;

    switch (property->insurance_type) {
        case INS_BASIC:                 return repair_cost * 80 / 100;
        case INS_COMPREHENSIVE:         return repair_cost;
        case INS_BUSINESS_INTERRUPTION: return repair_cost + property->current_rent * 10 * 5;
        default: return 0;
    }
}


void random_disaster(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    int pos = random_owned_property(monopoly, 1);
    if (pos < 0) return;

    int disaster = rand() % 5;
    struct Property_S *property = &board[pos].properties;
    struct Player_S *owner = &monopoly->players[property->current_owner];

    int repair_cost = property->house_count * property->house_construction_cost / 2;
    repair_cost += property->hotel_count * property->hotel_construction_cost / 2;
    if (repair_cost <= 0) return;

    property->is_damaged = 1;
    property->repair_cost = repair_cost;

    printf("%s occurred.\n", disaster_name(disaster));
    printf("Affected Property :\n%s.\n", board[pos].name);

    int covered = 0;
    if (property->insurance_type == INS_COMPREHENSIVE) covered = 1;
    else if (property->insurance_type == INS_BUSINESS_INTERRUPTION) covered = 1;
    else if (property->insurance_type == INS_BASIC && (disaster == DIS_FIRE || disaster == DIS_FLOOD)) covered = 1;

    if (covered) {
        int payout = insurance_payout(monopoly, pos, repair_cost);
        owner->cash += payout;
        printf("Insurance Claim Approved.\n");
        printf("Compensation Paid :\nLKR %d.\n", payout);
    }
    else {
        printf("No valid insurance policy.\n");
    }

    repair_property(monopoly, pos);
}


void repair_property(struct Monopoly_S *monopoly, int pos) {
    struct Square_S *sq = &monopoly->board[pos];
    struct Property_S *property = &sq->properties;

    if (property->is_damaged == 0) return;
    if (property->current_owner == PL_NONE) return;

    struct Player_S *owner = &monopoly->players[property->current_owner];
    if (owner->cash < property->repair_cost) return;

    owner->cash -= property->repair_cost;
    property->is_damaged = 0;

    for (int i = 0; i < property->house_count; i++) property->house_condition[i] = 100;
    if (property->hotel_count) property->hotel_condition = 100;

    printf("%s repaired %s for LKR %d.\n", owner->name, sq->name, property->repair_cost);
    property->repair_cost = 0;
}


void maintain_buildings(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->house_count == 0 && property->hotel_count == 0) continue;

        if (eval_maintain_building(monopoly, property) == 0) continue;

        int cost = 0;
        if (property->hotel_count) cost = property->hotel_construction_cost * 8 / 100;
        else cost = property->house_count * property->house_construction_cost * 5 / 100;

        cost = cost * property->maintenance_cost_mult / 100;
        if (cost > player->cash) continue;

        player->cash -= cost;
        for (int j = 0; j < property->house_count; j++) property->house_condition[j] = 100;
        if (property->hotel_count) property->hotel_condition = 100;
        property->missed_maintenance = 0;

        printf("%s maintained %s for LKR %d.\n", player->name, sq->name, cost);
    }
}


const char *disaster_name(int disaster) {
    switch (disaster) {
        case DIS_FIRE:               return "Fire";
        case DIS_FLOOD:              return "Flood";
        case DIS_RIOT:               return "Riot";
        case DIS_BUILDING_COLLAPSE:  return "Building Collapse";
        case DIS_ELECTRICAL_FAILURE: return "Electrical Failure";
    }
    return "None";
}
