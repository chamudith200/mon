#include "types.h"
#include "game.h"
#include "strat.h"
#include "player.h"
#include "events.h"

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
    player->property_owned = 0;
    player->hotels_owned = 0;

    player->property_val = 0;
    player->building_val = 0;
    player->railway_val = 0;
    player->utilitty_val = 0;
    player->insurance_claim_recivable = 0;
    player->taxes_due = 0;

    player->net_worth = 0;
}


void player_move(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    int d1 = roll_dice();
    int d2 = roll_dice();
    int dice = d1 + d2;
    int is_double = (d1 == d2);

    if (player->in_jail) {
        if (player->jail_turns == 0){
            player->in_jail = 0;
            printf("%s left Jail after 3 turns.\n\n", player->name);
        }
        else if(is_double) {
            player->in_jail = 0;
            player->jail_turns = 0;
            printf("%s rolled doubles and left Jail.\n\n", player->name);
        }    
        else if (player->cash >= 300) {
            player->in_jail = 0;
            player->jail_turns = 0;
            printf("%s paid bail of LKR 300.\n\n", player->name);
        }
        else {
            player->jail_turns--;
            return; 
        }
    }

    int current_pos = player->current_pos;
    int new_pos = (player->current_pos + dice) % 40;
    printf("%s rolled %d.\n", player->name, dice);
    printf("%s moved from Square %d to square %d.\n", player->name, player->current_pos, new_pos);

    if (current_pos + dice >= 40) {
        player->passed_go++;
        player->cash += 2000;
        printf("%s passed GO\n", player->name);
        printf("Collected LKR 2000.\n");
        printf("Current Balance : LKR %d.\n", player->cash);
    }
    player->current_pos = new_pos;
}


void player_buy_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    printf("%s purchased %s for LKR %d\n", player->name, sq->name, sq->properties.purchase_price);
    player->cash -= sq->properties.purchase_price;
    printf("Remaining Balance : LKR %d\n", player->cash);
    sq->properties.current_owner = player->id;
    player->property_owned++;
}


void player_property_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *prop = &monopoly->board[player->current_pos].properties;
    struct Player_S *owner = &monopoly->players[prop->current_owner];

    if (prop->closed_rounds > 0) {
        printf("%s is closed. No rent collected.\n", monopoly->board[player->current_pos].name);
        return;
    }

    int rent_multiplier = 1;
    switch (prop->house_count) {
        case 1: rent_multiplier = 2; break;
        case 2: rent_multiplier = 3; break;
        case 3: rent_multiplier = 5; break;
        case 4: rent_multiplier = 7; break;
    }

    int rent = prop->current_rent * rent_multiplier;
    rent = rent * monopoly->game_state.ee_mult.rent_mult / 100;
    rent = rent * property_rent_mult(monopoly, player->current_pos) / 100;

    if (prop->hotel_count == 1)  {
        rent = prop->current_rent * 10;
        rent = rent * monopoly->game_state.ee_mult.hotel_rent_mult / 100;
        rent = rent * property_rent_mult(monopoly, player->current_pos) / 100;
        if (owner->id == monopoly->game_state.national_event_player) {
            rent = rent * monopoly->game_state.ne_mult.hotel_rent_mult / 100;
        }
    }

    int avg_condition = 100;
    if (prop->house_count > 0) {
        avg_condition = 0;
        for (int i = 0; i < prop->house_count; i++) {
            avg_condition += prop->house_condition[i];
        }
        avg_condition /= prop->house_count;
    }
    if (prop->hotel_count == 1){
        avg_condition = prop->hotel_condition;
    }

    int condition_multiplier = 100;
    if (avg_condition >= 90) condition_multiplier = 100;
    else if (avg_condition >= 75) condition_multiplier = 90;
    else if (avg_condition >= 50) condition_multiplier = 75;
    else if (avg_condition >= 25) condition_multiplier = 50;
    else condition_multiplier = 0;

    rent = rent * condition_multiplier / 100;


    if (player->cash < rent) {
        int required = rent - player->cash;
        int raised = attempt_raise_cash(monopoly, required);
        if (raised == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d\n", rent);
    printf("Owner : %s\n", owner->name);
}


void player_buy_util_railway(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    printf("%s purchased %s for LKR %d\n", player->name, sq->name, sq->properties.purchase_price);
    player->cash -= sq->properties.purchase_price;
    printf("Remaining Balance : LKR %d\n", player->cash);
    sq->properties.current_owner = player->id;
    player->railway_owned++;
}


void player_railway_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *rail = &monopoly->board[player->current_pos].properties;
    struct Player_S *owner = &monopoly->players[rail->current_owner];

    int rent = 0;
    switch(owner->railway_owned) {
        case 1: rent =  250; break;
        case 2: rent =  500; break;
        case 3: rent = 1000; break;
        case 4: rent = 2000; break;
    }

    rent = rent * monopoly->game_state.ee_mult.rail_rent_mult / 100;
    rent = rent * monopoly->game_state.rd_mult.rail_rent_mult / 100;
    if (monopoly->game_state.active_gov_reg == GR_RAILWAY_MODERNIZATION) rent = rent * 125 / 100;
    if (owner->id == monopoly->game_state.national_event_player) {
        rent = rent * monopoly->game_state.ne_mult.rail_rent_mult / 100;
    }

    if (player->cash < rent) {
        int required = rent - player->cash;
        int raised = attempt_raise_cash(monopoly, required);
        if (raised == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    
    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d\n", rent);
    printf("Owner : %s\n", owner->name);
}


void player_util_pay_rent(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Property_S *util = &monopoly->board[player->current_pos].properties;
    struct Player_S *owner = &monopoly->players[util->current_owner];

    int rent = 0;
    switch(owner->util_owned) {
        case 1: rent =  4 * (roll_dice() + roll_dice()); break;
        case 2: rent = 10 * (roll_dice() + roll_dice()); break;
    }

    rent = rent * monopoly->game_state.rd_mult.util_rent_mult / 100;
    if (monopoly->game_state.active_gov_reg == GR_ELEC_TARRIF_REVISION) rent = rent * 120 / 100;
    if (owner->id == monopoly->game_state.national_event_player) {
        rent = rent * monopoly->game_state.ne_mult.util_income_mult / 100;
    }

    if (player->cash < rent) {
        int required = rent - player->cash;
        int raised = attempt_raise_cash(monopoly, required);
        if (raised == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }
    
    player->cash -= rent;
    owner->cash += rent;

    printf("Rent Paid : LKR %d\n", rent);
    printf("Owner : %s\n", owner->name);
}


void player_go_to_jail(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    player->current_pos = BOARD_JAIL_VISITING;
    player->in_jail = 1;
    player->jail_turns = 3;
    
}


void player_develop_house(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int dev_cost = sq->properties.house_construction_cost;
    dev_cost = dev_cost * monopoly->game_state.ee_mult.prop_dev_cost_mult / 100;
    dev_cost = dev_cost * monopoly->game_state.ee_mult.house_contruct_cost_mult / 100;
    if (player->id == monopoly->game_state.national_event_player) {
        dev_cost = dev_cost * monopoly->game_state.ne_mult.house_construct_cost_mult / 100;
        dev_cost = dev_cost * monopoly->game_state.ne_mult.construct_cost_mult / 100;
    }
    if (monopoly->game_state.active_gov_reg == GR_HOUSING_SUBSIDY) dev_cost = dev_cost * 70 / 100;

    player->cash -= dev_cost;
    sq->properties.house_count++;
    
    printf("%s constructed ", player->name);
    switch (sq->properties.house_count) {
        case 1: printf("one house");    break;
        case 2: printf("two houses");   break;
        case 3: printf("three houses"); break;
        case 4: printf("four houses");  break;
    }
    printf(" on %s, %d\n", sq->name, dev_cost);
}


void player_develop_hotel(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    int dev_cost = sq->properties.hotel_construction_cost;
    dev_cost = dev_cost * monopoly->game_state.ee_mult.prop_dev_cost_mult / 100;
    if (player->id == monopoly->game_state.national_event_player) {
        dev_cost = dev_cost * monopoly->game_state.ne_mult.construct_cost_mult / 100;
    }

    player->cash -= dev_cost;
    sq->properties.house_count = 0;
    sq->properties.hotel_count = 1;
    player->hotels_owned++;

    printf("%s upgraded %s to a Hotel, %d\n", player->name, sq->name, dev_cost);

}


int player_max_loan_amount(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct Square_S *board = monopoly->board;

    int mortgadge_val = 0;

    for (int i = 0 ; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type == ST_PROPERTY || sq->type == ST_RAILWAY || sq->type == ST_UTILITY) {
            if (sq->properties.current_owner != player->id) continue;
            if (sq->properties.is_mortaged) continue;
            mortgadge_val += sq->properties.mortgage_val;
        }
    }

    int max_loan = mortgadge_val * 75 / 100;
    return max_loan;
}


void player_renovate_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *property = &sq->properties;

    int ren_cost = property->purchase_price / 10;
    player->cash -= ren_cost;
    property->depreciation_precent = 0;
    printf("AGE: %d - ", property->property_age);
    property->property_age = 0;
    printf("%s renovated %s for LKR %d.\n", player->name, sq->name, ren_cost);
}


void player_pay_taxes(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    calc_networth(monopoly);
    int total_assests = player->cash + player->property_val + player->utilitty_val + player->railway_val;
    int tax_rate = monopoly->game_state.income_tax_rate;
    int tax = total_assests * tax_rate / 100;    

    if (player->cash < tax) {
        int required = tax - player->cash;
        int raised = attempt_raise_cash(monopoly, required);
        if (raised == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }
    // pay logic
    player->cash -= tax;
    printf("%s payed LKR %d in taxes.\n", player->name, tax);
}   


void  player_pay_community_fund(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    calc_networth(monopoly);
    int total_assests = player->cash + player->property_val + player->utilitty_val + player->railway_val;
    int tax_rate = monopoly->game_state.community_dev_fund_tax_rate;
    int tax = total_assests * tax_rate / 100;   
    
    if (player->cash < tax) {
        int required = tax - player->cash;
        int raised = attempt_raise_cash(monopoly, required);
        if (raised == 0) {
            eliminate_player(monopoly, player);
            return;
        }
    }

    // pay logic
    player->cash -= tax;
    printf("%s payed LKR %d in community funds.\n", player->name, tax);
}

/*
0- not possible
else -  the amount raised
*/
int attempt_raise_cash(struct Monopoly_S *monopoly, int required_min) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    if (player->property_owned == 0 && player->railway_owned == 0 && player->util_owned == 0) return 0;


    struct Property_S *min_property = NULL;
    struct Square_S *min_sq = NULL;
    struct Square_S *board = monopoly->board;
    for (int i = 0; i < 40; i++) {
        if (board[i].type != ST_PROPERTY && board[i].type != ST_RAILWAY && board[i].type != ST_UTILITY) continue;
        struct Property_S *property = &board[i].properties;
        if (property->current_owner != player->id) continue;   
        
        if (min_property == NULL) {
            if (property->purchase_price / 2 >= required_min) {
                min_property = property;
                min_sq = &board[i];
                continue;
            }
            else continue;
        }
        
        if (property->purchase_price / 2 < min_property->purchase_price / 2) {
            min_property = property;
            min_sq = &board[i];
            continue;
        }
    }
    if (min_property == NULL) return 0;
    
    int selling_price = min_property->purchase_price / 2;
    player->cash += selling_price;
    switch(min_sq->type) {
        case ST_PROPERTY: 
            if (player->property_owned > 0) player->property_owned--; 
            break;
        case ST_RAILWAY:  
            if (player->railway_owned > 0) player->railway_owned--;  
            break;
        case ST_UTILITY:   
            if (player->util_owned > 0) player->util_owned--;     
            break;
        default: break;
    }
    min_property->current_owner = PL_NONE;
    
    min_property->house_count = 0;
    for (int i = 0; i < 4; i++) min_property->house_condition[i] = 100;
    min_property->hotel_condition = 0;
    min_property->hotel_condition = 0;
    
    printf("%s sold %s for LKR %d.\n", player->name, min_sq->name, selling_price);
    return selling_price;
    
}


void eliminate_player(struct Monopoly_S *monopoly, struct Player_S *player) {
    if (player->is_bankrupt) return; // Prevent double elimination

    player->is_bankrupt = 1;
    player->cash = 0;
    player->has_active_loan = 0;
    player->loan_amount = 0;
    player->accrued_interest = 0;
    player->railway_owned = 0;
    player->util_owned = 0;
    player->property_owned = 0;
    player->hotels_owned = 0;
    player->net_worth = 0;

    printf("%s has been declared bankrupt.\n", player->name);
    printf("Remaining assets transferred to the Bank.\n");

    struct Square_S *board = monopoly->board;
    
    // Store properties to auction after state cleanup
    int properties_to_auction[40];
    int auction_count = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY && sq->type != ST_RAILWAY && sq->type != ST_UTILITY) continue;
        struct Property_S *property = &sq->properties;

        if (property->current_owner == player->id) {
            // Reset property attributes
            for (int j = 0; j < 4; j++) property->house_condition[j] = 100;
            property->hotel_condition = 100;
            property->house_count = 0;
            property->hotel_count = 0;
            property->is_mortaged = 0;
            property->is_loan_locked = 0;   
            property->insurance_type = INS_NONE;
            property->insuarance_rounds = 0;
            property->closed_rounds = 0;
            
            property->current_owner = PL_NONE;

            // Queue for auction
            properties_to_auction[auction_count++] = i;
        }
    }

    monopoly->game_state.bankrupt_count++;

    // Auction properties AFTER player state is fully cleared
    for (int k = 0; k < auction_count; k++) {
        auction(monopoly, &board[properties_to_auction[k]]);
        putchar('\n');
    }
}