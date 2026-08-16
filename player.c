#include "player.h"
#include "types.h"
#include "game.h"
#include "board.h"
#include "strat.h"
#include "events.h"
#include "finance.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



void player_init(struct Player_S *player, int id, enum PlayerType_E type, const char *name) {
    player->id = id;
    strncpy(player->name, name, sizeof(player->name) - 1);
    player->type = type;

    player->current_pos = BOARD_GO;
    player->is_bankrupt = 0;
    player->in_jail = 0;
    player->jail_turns = 0;
    player->passed_go = 0;

    player->cash = 30000;

    player->has_active_loan = 0;
    player->loan_amount = 0;
    player->accrued_interest = 0;
    player->loan_interest_rate = 0;
    player->loan_round_remaining = 0;

    player->railway_owned = 0;
    player->util_owned = 0;
    player->property_owned = 0;
    player->hotels_owned = 0;

    player->property_val = 0;
    player->building_val = 0;
    player->railway_val = 0;
    player->utilitty_val = 0;
    player->insurance_claim_recivable = 0;
    player->taxes_due = 0;

    player->suffered_loss = 0;

    player->net_worth = 0;
}



void player_buy_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];

    int price = sq->properties.purchase_price;
    player->cash -= price;
    sq->properties.current_owner = player->id;
    player->property_owned++;

    printf("%s purchased %s for LKR %d.\n", player->name, sq->name, price);
    printf("Remaining Balance : LKR %d.\n", player->cash);
}



void player_property_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *prop = &sq->properties;
    struct Player_S *owner = &monopoly->players[prop->current_owner];

    if (prop->is_mortaged) {
        printf("%s is mortgaged. No rent collected.\n", sq->name);
        return;
    }

    if (prop->closed_rounds > 0) {
        printf("%s is closed. No rent collected.\n", sq->name);
        return;
    }

    if (prop->is_damaged) {
        printf("%s is damaged. No rent collected.\n", sq->name);
        return;
    }

    int rent = prop->current_rent;
    switch (prop->house_count) {
        case 1: rent = prop->current_rent * 2; break;
        case 2: rent = prop->current_rent * 3; break;
        case 3: rent = prop->current_rent * 5; break;
        case 4: rent = prop->current_rent * 7; break;
    }

    if (prop->hotel_count == 1) {
        rent = prop->current_rent * 10;
        rent = rent * event_hotel_rent_mult(monopoly, owner) / 100;
    }

    rent = rent * event_rent_mult(monopoly, sq) / 100;
    rent = rent * (100 - prop->depreciation_precent) / 100;

    int avg_condition = 100;
    if (prop->house_count > 0) {
        avg_condition = 0;
        for (int i = 0; i < prop->house_count; i++) {
            avg_condition += prop->house_condition[i];
        }
        avg_condition /= prop->house_count;
    }
    if (prop->hotel_count == 1) avg_condition = prop->hotel_condition;

    int condition_multiplier = 100;
    if (avg_condition >= 90) condition_multiplier = 100;
    else if (avg_condition >= 75) condition_multiplier = 90;
    else if (avg_condition >= 50) condition_multiplier = 75;
    else if (avg_condition >= 25) condition_multiplier = 50;
    else condition_multiplier = 0;

    rent = rent * condition_multiplier / 100;
    if (rent <= 0) {
        printf("%s is closed for maintenance. No rent collected.\n", sq->name);
        return;
    }

    if (player->cash < rent) {
        int required = rent - player->cash;
        if (attempt_raise_cash(monopoly, required) == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d.\n", rent);
    printf("Owner : %s.\n", owner->name);
}



void player_buy_util_railway(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];

    int price = sq->properties.purchase_price;
    player->cash -= price;
    sq->properties.current_owner = player->id;

    if (sq->type == ST_RAILWAY) player->railway_owned++;
    else player->util_owned++;

    printf("%s purchased %s for LKR %d.\n", player->name, sq->name, price);
    printf("Remaining Balance : LKR %d.\n", player->cash);
}



void player_railway_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *rail = &sq->properties;
    struct Player_S *owner = &monopoly->players[rail->current_owner];

    if (rail->is_mortaged) {
        printf("%s is mortgaged. No rent collected.\n", sq->name);
        return;
    }

    int rent = 0;
    switch (owner->railway_owned) {
        case 1: rent =  250; break;
        case 2: rent =  500; break;
        case 3: rent = 1000; break;
        case 4: rent = 2000; break;
    }

    rent = rent * event_rail_rent_mult(monopoly, owner) / 100;

    if (player->cash < rent) {
        int required = rent - player->cash;
        if (attempt_raise_cash(monopoly, required) == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d.\n", rent);
    printf("Owner : %s.\n", owner->name);
}



void player_util_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *util = &sq->properties;
    struct Player_S *owner = &monopoly->players[util->current_owner];

    if (util->is_mortaged) {
        printf("%s is mortgaged. No rent collected.\n", sq->name);
        return;
    }

    int rent = 0;
    switch (owner->util_owned) {
        case 1: rent =  4 * (roll_dice() + roll_dice()); break;
        case 2: rent = 10 * (roll_dice() + roll_dice()); break;
    }

    rent = rent * event_util_rent_mult(monopoly, owner) / 100;

    if (player->cash < rent) {
        int required = rent - player->cash;
        if (attempt_raise_cash(monopoly, required) == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d.\n", rent);
    printf("Owner : %s.\n", owner->name);
}



void player_go_to_jail(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    player->current_pos = BOARD_JAIL_VISITING;
    player->in_jail = 1;
    player->jail_turns = 3;

    printf("%s was sent to Jail.\n", player->name);
}



void player_develop_house(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int dev_cost = sq->properties.house_construction_cost;
    dev_cost = dev_cost * event_house_cost_mult(monopoly, player) / 100;
    if (dev_cost > player->cash) return;

    player->cash -= dev_cost;
    sq->properties.house_count++;
    sq->properties.house_condition[sq->properties.house_count - 1] = 100;

    printf("%s constructed ", player->name);
    switch (sq->properties.house_count) {
        case 1: printf("one house");    break;
        case 2: printf("two houses");   break;
        case 3: printf("three houses"); break;
        case 4: printf("four houses");  break;
    }
    printf(" on %s.\n", sq->name);
    printf("Construction Cost : LKR %d.\n", dev_cost);
}



void player_develop_hotel(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int dev_cost = sq->properties.hotel_construction_cost;
    dev_cost = dev_cost * event_hotel_cost_mult(monopoly, player) / 100;
    if (dev_cost > player->cash) return;

    player->cash -= dev_cost;
    sq->properties.house_count = 0;
    sq->properties.hotel_count = 1;
    sq->properties.hotel_condition = 100;
    player->hotels_owned++;

    printf("%s upgraded %s to a Hotel.\n", player->name, sq->name);
    printf("Construction Cost : LKR %d.\n", dev_cost);
}



void eliminate_player(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct Square_S *board = monopoly->board;

    if (player->is_bankrupt) return;

    player->is_bankrupt = 1;
    player->cash = 0;
    player->has_active_loan = 0;
    player->loan_amount = 0;
    player->accrued_interest = 0;
    player->loan_round_remaining = 0;
    player->railway_owned = 0;
    player->util_owned = 0;
    player->property_owned = 0;
    player->hotels_owned = 0;
    player->net_worth = 0;

    printf("%s has been declared bankrupt.\n", player->name);
    printf("Remaining assets transferred to the Bank.\n");

    int to_auction[40];
    int auction_count = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;

        for (int j = 0; j < 4; j++) property->house_condition[j] = 100;
        property->hotel_condition = 100;
        property->house_count = 0;
        property->hotel_count = 0;
        property->is_mortaged = 0;
        property->is_loan_locked = 0;
        property->is_damaged = 0;
        property->repair_cost = 0;
        property->missed_maintenance = 0;
        property->maintenance_cost_mult = 100;
        property->closed_rounds = 0;
        property->insurance_type = INS_NONE;
        property->insuarance_rounds = 0;
        property->current_owner = PL_NONE;

        to_auction[auction_count++] = i;
    }

    monopoly->game_state.bankrupt_count++;

    for (int i = 0; i < auction_count; i++) {
        auction(monopoly, &board[to_auction[i]]);
        putchar('\n');
    }
}
