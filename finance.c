#include "finance.h"
#include "types.h"
#include "game.h"
#include "player.h"
#include "strat.h"
#include "events.h"

#include <stdio.h>
#include <stdlib.h>


/*
recalculates the current player net worth
*/
void calc_networth(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    int prop_val = 0;
    int building_val = 0;
    int rail_val = 0;
    int util_val = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;

        if (sq->type == ST_PROPERTY) {
            prop_val += property_value(monopoly, sq);
            building_val += property->house_count * property->house_construction_cost;
            building_val += property->hotel_count * property->hotel_construction_cost;
        }
        else if (sq->type == ST_RAILWAY) rail_val += property_value(monopoly, sq);
        else if (sq->type == ST_UTILITY) util_val += property_value(monopoly, sq);
    }

    player->property_val = prop_val;
    player->building_val = building_val;
    player->railway_val = rail_val;
    player->utilitty_val = util_val;

    player->net_worth = player->cash + player->property_val + player->building_val + player->railway_val
                        + player->utilitty_val + player->insurance_claim_recivable
                        - player->loan_amount - player->accrued_interest - player->taxes_due;
}


/*
rolls a new inflation rate every 10 rounds and compounds it into every price
*/
void apply_inflation(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Square_S *board = monopoly->board;

    game_state->current_interest_rate = economy_interest_rate(monopoly);
    if (game_state->current_round % 10 != 0) return;

    int inflation_dict[] = {-3, 0, 2, 5, 8, 12};
    game_state->current_inflation_rate = inflation_dict[rand() % 6];

    int inflation = game_state->current_inflation_rate;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        struct Property_S *property = &sq->properties;

        switch (sq->type) {
            case ST_PROPERTY:
                property->market_price += property->market_price * inflation / 100;
                property->purchase_price += property->purchase_price * inflation / 100;
                property->mortgage_val += property->mortgage_val * inflation / 100;
                property->house_construction_cost += property->house_construction_cost * inflation / 100;
                property->hotel_construction_cost += property->hotel_construction_cost * inflation / 100;
                property->current_rent += property->current_rent * inflation / 100;
                property->repair_cost += property->repair_cost * inflation / 100;
                break;

            case ST_RAILWAY:
            case ST_UTILITY:
                property->market_price += property->market_price * inflation / 100;
                property->purchase_price += property->purchase_price * inflation / 100;
                property->mortgage_val += property->mortgage_val * inflation / 100;
                break;

            default:
                break;
        }
    }
}


/*
ages every property, wears down buildings and applies structural damage
*/
void prop_building_condition(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;

        property->property_age++;
        if (property->property_age > 50 && property->property_age % 5 == 0 && property->depreciation_precent < 30) {
            property->depreciation_precent++;
            property->market_price -= property->market_price / 100;

            printf("Property\n%s\nhas depreciated by %d%%.\n", sq->name, property->depreciation_precent);
            printf("Current Value\nLKR %d.\n", property_value(monopoly, sq));
        }

        if (property->hotel_count) {
            property->hotel_condition -= 2;
            if (property->hotel_condition < 0) property->hotel_condition = 0;
        }

        for (int j = 0; j < property->house_count; j++) {
            property->house_condition[j] -= 2;
            if (property->house_condition[j] < 0) property->house_condition[j] = 0;
        }

        if (property->house_count == 0 && property->hotel_count == 0) continue;

        int condition = property->hotel_count ? property->hotel_condition : property->house_condition[0];
        if (condition < 100) property->missed_maintenance++;
        else property->missed_maintenance = 0;

        if (property->missed_maintenance > 20 && property->is_damaged == 0) {
            property->is_damaged = 1;
            property->market_price -= property->market_price * 15 / 100;
            property->current_rent -= property->current_rent * 25 / 100;
            property->maintenance_cost_mult = 150;
            property->repair_cost = property->market_price / 4;
            property->missed_maintenance = 0;
            if (property->current_owner != PL_NONE) monopoly->players[property->current_owner].suffered_loss = 1;

            printf("Structural damage on %s.\n", sq->name);
        }
    }
}


/*
clears depreciation and age on the square the current player stands on
*/
void player_renovate_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *property = &sq->properties;

    int ren_cost = property_value(monopoly, sq) / 10;
    if (property->is_damaged) ren_cost = property_value(monopoly, sq) / 4;
    if (player->cash < ren_cost) return;

    player->cash -= ren_cost;
    property->depreciation_precent = 0;
    property->property_age = 0;
    property->is_damaged = 0;
    property->repair_cost = 0;
    property->maintenance_cost_mult = 100;
    property->missed_maintenance = 0;

    for (int i = 0; i < property->house_count; i++) property->house_condition[i] = 100;
    if (property->hotel_count) property->hotel_condition = 100;

    printf("%s renovated %s for LKR %d.\n", player->name, sq->name, ren_cost);
}


/*
current bank interest rate for the prevailing economy
*/
int economy_interest_rate(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    int inflation = game_state->current_inflation_rate;
    int rate = 8;

    if (inflation < 0) rate = 5;
    else if (inflation <= 2) rate = 8;
    else if (inflation <= 5) rate = 10;
    else if (inflation <= 8) rate = 12;
    else rate = 15;

    if (game_state->active_economic_event == EE_ECONOMIC_RECESSION) rate = 15;

    rate = rate * game_state->ee_mult.loan_interest_mult / 100;
    rate += game_state->ne_mult.loan_interest_change;
    if (game_state->active_gov_reg == GR_REDUCE_LOAN_INTEREST) rate -= 2;
    if (rate < 1) rate = 1;

    return rate;
}


/*
total mortgage value of everything a player can still pledge
*/
int player_max_loan_amount(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct Square_S *board = monopoly->board;

    int mortgadge_val = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        if (sq->properties.current_owner != player->id) continue;
        if (sq->properties.is_mortaged) continue;
        if (sq->properties.is_loan_locked) continue;
        mortgadge_val += sq->properties.mortgage_val;
    }

    return mortgadge_val * 75 / 100;
}


/*
takes a secured loan and locks the pledged collateral
*/
void player_take_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    int max_loan = player_max_loan_amount(monopoly, player);
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


/*
pays down interest first then the principal
*/
void player_repay_loan(struct Monopoly_S *monopoly, int amount) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    if (player->has_active_loan == 0) return;
    if (amount > player->cash) amount = player->cash;

    int outstanding = player->loan_amount + player->accrued_interest;
    if (amount > outstanding) amount = outstanding;
    if (amount <= 0) return;

    player->cash -= amount;
    printf("%s repaid LKR %d.\n", player->name, amount);

    if (amount >= player->accrued_interest) {
        amount -= player->accrued_interest;
        player->accrued_interest = 0;
        player->loan_amount -= amount;
    }
    else {
        player->accrued_interest -= amount;
    }

    if (player->loan_amount + player->accrued_interest <= 0) {
        player->has_active_loan = 0;
        player->loan_amount = 0;
        player->accrued_interest = 0;
        player->loan_round_remaining = 0;

        for (int i = 0; i < 40; i++) {
            if (board[i].properties.current_owner == player->id) board[i].properties.is_loan_locked = 0;
        }
    }

    printf("Outstanding Balance :\nLKR %d.\n", player->loan_amount + player->accrued_interest);
}


/*
adds 10 more rounds to the loan term
*/
void player_extend_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan == 0) return;

    player->loan_round_remaining += 10;

    printf("%s extended the loan period.\n", player->name);
    printf("Duration : %d Rounds\n", player->loan_round_remaining);
}


/*
borrows more against any collateral still free
*/
void player_increase_loan(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    if (player->has_active_loan == 0) return;

    int extra = player_max_loan_amount(monopoly, player);
    if (extra <= 0) return;

    player->loan_amount += extra;
    player->cash += extra;

    printf("%s increased the loan by LKR %d.\n", player->name, extra);
    printf("Collateral :\n");

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        if (sq->properties.current_owner != player->id) continue;
        if (sq->properties.is_mortaged) continue;
        if (sq->properties.is_loan_locked) continue;
        sq->properties.is_loan_locked = 1;
        printf("%s\n", sq->name);
    }

    printf("Outstanding Balance :\nLKR %d.\n", player->loan_amount + player->accrued_interest);
}


/*
compounds interest on every loan and defaults the expired ones
*/
void accrue_loan_interest(struct Monopoly_S *monopoly) {
    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[i];
        if (player->is_bankrupt) continue;
        if (player->has_active_loan == 0) continue;

        int outstanding = player->loan_amount + player->accrued_interest;
        player->accrued_interest += outstanding * player->loan_interest_rate / 100;

        player->loan_round_remaining--;
        if (player->loan_round_remaining <= 0) loan_default(monopoly, player);
    }
}


/*
forecloses all pledged collateral and clears the debt
*/
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
        property->is_mortaged = 0;
        property->is_damaged = 0;
        property->current_owner = PL_NONE;

        switch (sq->type) {
            case ST_PROPERTY: if (player->property_owned > 0) player->property_owned--; break;
            case ST_RAILWAY:  if (player->railway_owned > 0)  player->railway_owned--;  break;
            case ST_UTILITY:  if (player->util_owned > 0)     player->util_owned--;     break;
            default: break;
        }
    }

    player->has_active_loan = 0;
    player->loan_amount = 0;
    player->accrued_interest = 0;
    player->loan_round_remaining = 0;
    player->suffered_loss = 1;

    printf("Outstanding debt cleared.\n");

    if (player->property_owned == 0 && player->railway_owned == 0 && player->util_owned == 0 && player->cash <= 0) {
        eliminate_player(monopoly, player);
    }
}


/*
premium for a policy on a square
*/
int insurance_premium(struct Monopoly_S *monopoly, struct Square_S *sq, enum InsuranceType_E type) {
    struct Property_S *property = &sq->properties;

    int value = property_value(monopoly, sq);
    value += property->house_count * property->house_construction_cost;
    value += property->hotel_count * property->hotel_construction_cost;

    int premium = 0;
    switch (type) {
        case INS_BASIC:                 premium = value *  5 / 100; break;
        case INS_COMPREHENSIVE:         premium = value * 10 / 100; break;
        case INS_BUSINESS_INTERRUPTION: premium = value * 15 / 100; break;
        default: return 0;
    }

    return premium * event_insurance_mult(monopoly) / 100;
}


/*
buys a policy for one property when the player lands on an insurer
*/
void player_buy_insurance(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    enum InsuranceType_E type = INS_NONE;
    struct Square_S *sq = eval_insurance_target(monopoly, &type);
    if (sq == NULL || type == INS_NONE) return;

    int premium = insurance_premium(monopoly, sq, type);
    if (premium <= 0 || premium > player->cash) return;

    player->cash -= premium;
    sq->properties.insurance_type = type;
    sq->properties.insuarance_rounds = 20;

    switch (type) {
        case INS_BASIC:                 printf("Basic Property Insurance purchased.\n");        break;
        case INS_COMPREHENSIVE:         printf("Comprehensive Insurance purchased.\n");         break;
        case INS_BUSINESS_INTERRUPTION: printf("Business Interruption Insurance purchased.\n"); break;
        default: break;
    }
    printf("Property : %s\n", sq->name);
    printf("Premium : LKR %d.\n", premium);
}


/*
counts down every policy and warns 3 rounds before expiry
*/
void insurance_tick(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        struct Property_S *property = &sq->properties;
        if (property->insurance_type == INS_NONE) continue;

        property->insuarance_rounds--;

        if (property->insuarance_rounds == 3) {
            printf("Insurance policy on %s expires in 3 rounds.\n", sq->name);
        }

        if (property->insuarance_rounds <= 0) {
            property->insurance_type = INS_NONE;
            property->insuarance_rounds = 0;
            printf("Insurance policy on %s has expired.\n", sq->name);
        }
    }
}


/*
1 if the policy on a square covers the given disaster
*/
int insurance_covers(struct Square_S *sq, int disaster) {
    switch (sq->properties.insurance_type) {
        case INS_BASIC:
            if (disaster == DIS_FIRE || disaster == DIS_FLOOD) return 1;
            return 0;

        case INS_COMPREHENSIVE:
            if (disaster == DIS_ELECTRICAL_FAILURE) return 0;
            return 1;

        case INS_BUSINESS_INTERRUPTION:
            if (sq->properties.hotel_count == 0) return 0;
            return 1;
    }
    return 0;
}


/*
compensation a policy pays out against a repair bill
*/
int insurance_payout(struct Monopoly_S *monopoly, struct Square_S *sq, int repair_cost) {
    struct Property_S *property = &sq->properties;

    int claim_mult = monopoly->game_state.ee_mult.business_interrupt_claim_mult;

    switch (property->insurance_type) {
        case INS_BASIC:
            return repair_cost * 80 / 100;

        case INS_COMPREHENSIVE:
            return repair_cost;

        case INS_BUSINESS_INTERRUPTION:
            return repair_cost + property->current_rent * 10 * 5 * claim_mult / 100;
    }
    return 0;
}


/*
hits one random developed property every 10 rounds
*/
void random_disaster(struct Monopoly_S *monopoly) {
    struct Square_S *sq = random_owned_property(monopoly, 1);
    if (sq == NULL) return;

    struct Property_S *property = &sq->properties;
    struct Player_S *owner = &monopoly->players[property->current_owner];

    int disaster = rand() % 6;
    if (disaster == DIS_RIOT && (rand() % 100) >= monopoly->game_state.ee_mult.riot_prob_mult / 2) return;

    int repair_cost = property->house_count * property->house_construction_cost / 2;
    repair_cost += property->hotel_count * property->hotel_construction_cost / 2;
    if (repair_cost <= 0) return;

    property->is_damaged = 1;
    property->repair_cost = repair_cost;
    owner->suffered_loss = 1;

    for (int i = 0; i < property->house_count; i++) property->house_condition[i] = 0;
    if (property->hotel_count) property->hotel_condition = 0;

    printf("%s occurred.\n", disaster_name(disaster));
    printf("Affected Property :\n%s.\n", sq->name);

    if (insurance_covers(sq, disaster)) {
        int payout = insurance_payout(monopoly, sq, repair_cost);
        owner->cash += payout;
        printf("Insurance Claim Approved.\n");
        printf("Compensation Paid :\nLKR %d.\n", payout);
    }
    else {
        printf("No valid insurance policy.\n");
    }

    repair_property(monopoly, sq);
}


/*
restores a damaged property once the owner can pay
*/
void repair_property(struct Monopoly_S *monopoly, struct Square_S *sq) {
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


/*
restores building condition at the start of a turn
*/
void maintain_buildings(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->house_count == 0 && property->hotel_count == 0) continue;

        if (property->is_damaged) {
            repair_property(monopoly, sq);
            continue;
        }

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


/*
charges income tax on the current player
*/
void player_pay_taxes(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    calc_networth(monopoly);
    int total_assests = player->cash + player->property_val + player->utilitty_val + player->railway_val;
    int tax = total_assests * monopoly->game_state.income_tax_rate / 100;
    if (tax < 0) tax = 0;

    if (player->cash < tax) {
        int required = tax - player->cash;
        if (attempt_raise_cash(monopoly, required) == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= tax;
    printf("%s payed LKR %d in taxes.\n", player->name, tax);
}


/*
charges the community development fund levy
*/
void player_pay_community_fund(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    calc_networth(monopoly);
    int total_assests = player->cash + player->property_val + player->utilitty_val + player->railway_val;
    int tax = total_assests * monopoly->game_state.community_dev_fund_tax_rate / 100;
    if (tax < 0) tax = 0;

    if (player->cash < tax) {
        int required = tax - player->cash;
        if (attempt_raise_cash(monopoly, required) == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= tax;
    printf("%s payed LKR %d in community funds.\n", player->name, tax);
}


/*
charges hotel owners the luxury tax while that regulation is active
*/
void luxury_property_tax(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    if (monopoly->game_state.active_gov_reg != GR_LUXURY_PROPERTY_TAX) return;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->hotel_count == 0) continue;
        if (property->current_owner == PL_NONE) continue;

        struct Player_S *owner = &monopoly->players[property->current_owner];
        int tax = (property_value(monopoly, sq) + property->hotel_construction_cost) * 25 / 100;
        if (tax <= 0) continue;

        if (owner->cash < tax) {
            int saved = monopoly->game_state.current_player;
            monopoly->game_state.current_player = owner->id;
            int raised = attempt_raise_cash(monopoly, tax - owner->cash);
            monopoly->game_state.current_player = saved;

            if (raised == 0) {
                eliminate_player(monopoly, owner);
                continue;
            }
        }

        owner->cash -= tax;

        printf("%s payed LKR %d luxury property tax on %s.\n", owner->name, tax, sq->name);
    }
}


/*
mortgages a square to the bank for its mortgage value
*/
int player_mortgage_property(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Property_S *property = &sq->properties;

    if (property->is_mortaged) return 0;
    if (property->is_loan_locked) return 0;
    if (property->current_owner == PL_NONE) return 0;

    struct Player_S *owner = &monopoly->players[property->current_owner];

    property->is_mortaged = 1;
    owner->cash += property->mortgage_val;

    printf("%s mortgaged %s for LKR %d.\n", owner->name, sq->name, property->mortgage_val);
    return property->mortgage_val;
}


/*
0 if impossible, otherwise the amount raised by mortgaging or selling
*/
int attempt_raise_cash(struct Monopoly_S *monopoly, int required_min) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    int raised = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->is_mortaged || property->is_loan_locked) continue;
        if (property->house_count || property->hotel_count) continue;

        raised += player_mortgage_property(monopoly, sq);
        if (raised >= required_min) return raised;
    }

    while (raised < required_min) {
        struct Square_S *min_sq = NULL;
        for (int i = 0; i < 40; i++) {
            struct Square_S *sq = &board[i];
            if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
            struct Property_S *property = &sq->properties;
            if (property->current_owner != player->id) continue;
            if (property->is_loan_locked) continue;

            if (min_sq == NULL) min_sq = sq;
            else if (property_value(monopoly, sq) < property_value(monopoly, min_sq)) min_sq = sq;
        }

        if (min_sq == NULL) break;

        int selling_price = property_value(monopoly, min_sq) / 2;
        player->cash += selling_price;
        raised += selling_price;

        switch (min_sq->type) {
            case ST_PROPERTY: if (player->property_owned > 0) player->property_owned--; break;
            case ST_RAILWAY:  if (player->railway_owned > 0)  player->railway_owned--;  break;
            case ST_UTILITY:  if (player->util_owned > 0)     player->util_owned--;     break;
            default: break;
        }

        struct Property_S *property = &min_sq->properties;
        if (property->hotel_count && player->hotels_owned > 0) player->hotels_owned--;
        property->current_owner = PL_NONE;
        property->house_count = 0;
        property->hotel_count = 0;
        property->is_mortaged = 0;
        property->is_damaged = 0;
        property->insurance_type = INS_NONE;
        property->insuarance_rounds = 0;
        for (int i = 0; i < 4; i++) property->house_condition[i] = 100;
        property->hotel_condition = 100;

        printf("%s sold %s for LKR %d.\n", player->name, min_sq->name, selling_price);
    }

    if (raised < required_min) return 0;
    return raised;
}


const char *disaster_name(int disaster) {
    switch (disaster) {
        case DIS_FIRE:               return "Fire";
        case DIS_FLOOD:              return "Flood";
        case DIS_RIOT:               return "Riot";
        case DIS_VANDALISM:          return "Vandalism";
        case DIS_BUILDING_COLLAPSE:  return "Building Collapse";
        case DIS_ELECTRICAL_FAILURE: return "Electrical Failure";
    }
    return "None";
}
