#include "game.h"
#include "types.h"
#include "player.h"
#include "board.h"
#include "strat.h"
#include "events.h"
#include "finance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TURNS 153
#define ROUNDS 500


void game_loop(struct Monopoly_S *monopoly) {
    first_player(monopoly);
    // int play_order[4];

    while (monopoly->game_state.current_round <= ROUNDS) {
    // while (monopoly->game_state.current_turn <= TURNS) {
        // before players play

        
        printf("\n----------Turn: %d----------\n", monopoly->game_state.current_turn);
        // for every player
        for (int i = 0; i < 4; i++) {
            if (monopoly->game_state.bankrupt_count >= 3) {
                round_summary(monopoly);
                end_game_summary(monopoly);
                return;
            }
            
            monopoly->game_state.current_player = monopoly->game_state.turn_order[monopoly->game_state.current_turn_order];
            struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
            
            if (player->is_bankrupt) {
                monopoly->game_state.current_turn_order = (monopoly->game_state.current_turn_order + 1) % 4;
                monopoly->game_state.current_turn++;
                continue;
            }

            before_roll(monopoly);
            
            putchar('\n');
            player_move(monopoly);
            player_land(monopoly);
            develope_property(monopoly);
            calc_networth(monopoly);
            check_bankrupt(monopoly);
            
            monopoly->game_state.current_turn++;    
            monopoly->game_state.current_turn_order = (monopoly->game_state.current_turn_order + 1) % 4;
        }
        
        // printf("\n----------------------------\n");
        if (is_round_passed(monopoly) != 0) {
            prop_building_condition(monopoly);
            apply_inflation(monopoly);
            draw_economic_event(monopoly);
            accrue_loan_interest(monopoly);
            insurance_tick(monopoly);
            expire_cards(monopoly);

            if (monopoly->game_state.current_round % 10 == 0) {
                market_review(monopoly);
                random_disaster(monopoly);
            }
            if (monopoly->game_state.current_round % 15 == 0) draw_regional_card(monopoly);
            if (monopoly->game_state.current_round % 20 == 0) draw_gov_regulation(monopoly);

            round_summary(monopoly);
            market_conditions(monopoly);
            
            monopoly->game_state.current_round++;
            for (int i = 0; i < 4; i++) {
                monopoly->players[i].passed_go--;
            }
        }
    }

    end_game_summary(monopoly);

}


void start_msg(struct Player_S players[4]) {
    printf("MONOPOLY-LK Simulation\n\n");
    for (int i = 0; i < 4; i++) {
        printf("Player %d : %s.\n", players[i].id ,players[i].name);
    }
    putchar('\n');
    printf("Each player begins with LKR 30,000.\n\n");
}


void init_players_all(struct Player_S players[4]) {
    player_init(&players[0], 0, PL_AGGRESIVE_INVESTOR, "Aggresive Investor");
    player_init(&players[1], 1, PL_CONSERVATIVE_BANKER, "Conservative Banker");
    player_init(&players[2], 2, PL_RISK_TAKER, "Risk Taker");
    player_init(&players[3], 3, PL_OPPORTUNISTIC_TRADER, "Opportunistic Trader");
}


void inti_squares_all(struct Square_S board[40]) {
    int id = 0;
    // 0
    board_init(&board[BOARD_GO], id++, "GO", ST_START);
    board_init(&board[BOARD_PETTAH], id++, "Pettah", ST_PROPERTY);
    board_init(&board[BOARD_COMMUNITY_FUND], id++, "Community Developement Fund", ST_EVENT);
    board_init(&board[BOARD_MARADANA], id++, "Maradana", ST_PROPERTY);
    board_init(&board[BOARD_INCOME_TAX], id++, "Income Tax", ST_TAX);
    // 5
    board_init(&board[BOARD_COLOMBO_FORT_STATION], id++, "Colombo Fort Railway Station", ST_RAILWAY);
    board_init(&board[BOARD_BAMBALAPITIYA], id++, "Bambalapitiya", ST_PROPERTY);
    board_init(&board[BOARD_NATIONAL_EVENT_1], id++, "National Event Card", ST_EVENT);
    board_init(&board[BOARD_WELLAWATTE], id++, "Wellawatte", ST_PROPERTY);
    board_init(&board[BOARD_MOUNT_LAVINIA], id++, "Mount Lavinia", ST_PROPERTY);
    //10
    board_init(&board[BOARD_JAIL_VISITING], id++, "Jail / Just Visiting", ST_SPECIAL);
    board_init(&board[BOARD_NUGEGODA], id++, "Nugegoda", ST_PROPERTY);
    board_init(&board[BOARD_CEB], id++, "Ceylon Electricity Board", ST_UTILITY);
    board_init(&board[BOARD_MAHARAGAMA], id++, "Maharagama", ST_PROPERTY);
    board_init(&board[BOARD_KOTTAWA], id++, "Kottawa", ST_PROPERTY);
    // 15
    board_init(&board[BOARD_KANDY_STATION], id++, "Kandy Railway Station", ST_RAILWAY);
    board_init(&board[BOARD_NEGOMBO], id++, "Negombo", ST_PROPERTY);
    board_init(&board[BOARD_SRI_LANKA_INSURANCE], id++, "Sri Lanka Insuarance", ST_INSURANCE);
    board_init(&board[BOARD_KATUNAYAKE], id++, "Katunayake", ST_PROPERTY);
    board_init(&board[BOARD_JA_ELA], id++, "Ja-Ela", ST_PROPERTY);
    // 20
    board_init(&board[BOARD_FREE_PARKING], id++, "Free Parking", ST_SPECIAL);
    board_init(&board[BOARD_KANDY_CITY], id++, "Kandy City", ST_PROPERTY);
    board_init(&board[BOARD_NATIONAL_EVENT_2], id++, "National Event Card", ST_EVENT);
    board_init(&board[BOARD_PERADENIYA], id++, "Peradeniya", ST_PROPERTY);
    board_init(&board[BOARD_KATUGASTOTA], id++, "Katugasthota", ST_PROPERTY);
    // 25
    board_init(&board[BOARD_GALLE_STATION], id++, "Galle Railway Station", ST_RAILWAY);
    board_init(&board[BOARD_GALLE_FORT], id++, "Galle Fort", ST_PROPERTY);
    board_init(&board[BOARD_UNAWATUNA], id++, "Unawatuna", ST_PROPERTY);
    board_init(&board[BOARD_NWSDB], id++, "National Water Supply and Drainage Board", ST_UTILITY);
    board_init(&board[BOARD_HIKKADUWA], id++, "Hikkauwa", ST_PROPERTY);
    // 30
    board_init(&board[BOARD_GO_TO_JAIL], id++, "GO To Jail", ST_SPECIAL);
    board_init(&board[BOARD_JAFFNA_TOWN], id++, "Jaffna Town", ST_PROPERTY);
    board_init(&board[BOARD_NALLUR], id++, "Nallur", ST_PROPERTY);
    board_init(&board[BOARD_CEYLINCO_INSURANCE], id++, "Ceylinco Insurance", ST_INSURANCE);
    board_init(&board[BOARD_TRINCOMALEE], id++, "Trincomalee", ST_PROPERTY);
    // 35
    board_init(&board[BOARD_JAFFNA_STATION], id++, "Jaffna Railway Station", ST_RAILWAY);
    board_init(&board[BOARD_NATIONAL_EVENT_3], id++, "National Event card", ST_EVENT);
    board_init(&board[BOARD_NUWARA_ELIYA], id++, "Nuwara Eliya", ST_PROPERTY);
    board_init(&board[BOARD_BANK_OF_CEYLON], id++, "Bank of Ceylon", ST_BANK);
    board_init(&board[BOARD_GALLE_FACE], id++, "Galle Face", ST_PROPERTY);
}


void init_propert_all(struct Square_S board[40]) {
    for (int i = 0; i < 40; i++) {
        if (board[i].type != ST_PROPERTY) {
            continue;
        }

        struct Property_S *property = &board[i].properties;
        property->current_owner = PL_NONE;

        property->is_mortaged = 0;
        property->is_loan_locked = 0;

        property->house_count = 0;
        property->hotel_count = 0;
        for (int j = 0; j < 4; j++) property->house_condition[j] = 100;
        property->hotel_condition = 100;
        property->property_age = 0;
        property->depreciation_precent = 1;

        property->insurance_type = INS_NONE;
        property->insuarance_rounds = 0;
        property->closed_rounds = 0;

        property->is_damaged = 0;
        property->repair_cost = 0;
        property->missed_maintenance = 0;
        property->maintenance_cost_mult = 100;

        if (i == BOARD_PETTAH || i == BOARD_MARADANA) {
            property->type = PT_BROWN;
            board[BOARD_PETTAH].properties.purchase_price = 1500;
            board[BOARD_PETTAH].properties.base_rental = 100;
            board[BOARD_MARADANA].properties.purchase_price = 1800;
            board[BOARD_MARADANA].properties.base_rental = 120;
            property->house_construction_cost = 500;
            property->hotel_construction_cost = 2000;
            property->mortgage_val = 750;
            property->base_price = 1500;

        }

        else if (i == BOARD_BAMBALAPITIYA || i == BOARD_WELLAWATTE || i == BOARD_MOUNT_LAVINIA) {
            property->type = PT_LIGHT_BLUE;
            board[BOARD_BAMBALAPITIYA].properties.purchase_price = 2500;
            board[BOARD_BAMBALAPITIYA].properties.base_rental = 180;
            board[BOARD_WELLAWATTE].properties.purchase_price = 2700;
            board[BOARD_WELLAWATTE].properties.base_rental = 200;
            board[BOARD_MOUNT_LAVINIA].properties.purchase_price = 3000;
            board[BOARD_MOUNT_LAVINIA].properties.base_rental = 220;
            property->house_construction_cost = 750;
            property->hotel_construction_cost = 3000;
            property->mortgage_val = 1250;
            property->base_price = 2500;
        }
        
        else if (i == BOARD_MAHARAGAMA || i == BOARD_NUGEGODA || i == BOARD_KOTTAWA) {
            property->type = PT_PINK;
            board[BOARD_NUGEGODA].properties.purchase_price = 3500;
            board[BOARD_NUGEGODA].properties.base_rental = 260;
            board[BOARD_MAHARAGAMA].properties.purchase_price = 3800;
            board[BOARD_MAHARAGAMA].properties.base_rental = 280;
            board[BOARD_KOTTAWA].properties.purchase_price = 4000;
            board[BOARD_KOTTAWA].properties.base_rental = 300;
            property->house_construction_cost = 1000;
            property->hotel_construction_cost = 4000;
            property->mortgage_val = 1750;
            property->base_price = 3500;
        }
        
        else if (i == BOARD_NEGOMBO || i == BOARD_KATUNAYAKE || i == BOARD_JA_ELA) {
            property->type = PT_ORANGE;
            board[BOARD_NEGOMBO].properties.purchase_price = 4500;
            board[BOARD_NEGOMBO].properties.base_rental = 350;
            board[BOARD_KATUNAYAKE].properties.purchase_price = 4700;
            board[BOARD_KATUNAYAKE].properties.base_rental = 370;
            board[BOARD_JA_ELA].properties.purchase_price = 5000;
            board[BOARD_JA_ELA].properties.base_rental = 400;
            property->house_construction_cost = 1250;
            property->hotel_construction_cost = 5000;
            property->mortgage_val = 2250;
            property->base_price = 4500;
        }
        
        else if (i == BOARD_KANDY_CITY || i == BOARD_PERADENIYA || i == BOARD_KATUGASTOTA) {
            property->type = PT_RED;
            board[BOARD_KANDY_CITY].properties.purchase_price = 5500;
            board[BOARD_KANDY_CITY].properties.base_rental = 450;
            board[BOARD_PERADENIYA].properties.purchase_price = 5800;
            board[BOARD_PERADENIYA].properties.base_rental = 480;
            board[BOARD_KATUGASTOTA].properties.purchase_price = 6000;
            board[BOARD_KATUGASTOTA].properties.base_rental = 500;
            property->house_construction_cost = 1500;
            property->hotel_construction_cost = 6000;
            property->mortgage_val = 2750;
            property->base_price = 5500;
        }
        
        else if (i == BOARD_GALLE_FORT || i == BOARD_UNAWATUNA || i == BOARD_HIKKADUWA) {
            property->type = PT_YELLOW;
            board[BOARD_GALLE_FORT].properties.purchase_price = 6500;
            board[BOARD_GALLE_FORT].properties.base_rental = 600;
            board[BOARD_UNAWATUNA].properties.purchase_price = 6800;
            board[BOARD_UNAWATUNA].properties.base_rental = 620;
            board[BOARD_HIKKADUWA].properties.purchase_price = 7000;
            board[BOARD_HIKKADUWA].properties.base_rental = 650;
            property->house_construction_cost = 2000;
            property->hotel_construction_cost = 8000;
            property->mortgage_val = 3250;
            property->base_price = 6500;
        }
        
        else if (i == BOARD_JAFFNA_TOWN || i == BOARD_NALLUR || i == BOARD_TRINCOMALEE) {
            property->type = PT_GREEN;
            board[BOARD_JAFFNA_TOWN].properties.purchase_price = 8000;
            board[BOARD_JAFFNA_TOWN].properties.base_rental = 750;
            board[BOARD_NALLUR].properties.purchase_price = 8300;
            board[BOARD_NALLUR].properties.base_rental = 780;
            board[BOARD_TRINCOMALEE].properties.purchase_price = 8500;
            board[BOARD_TRINCOMALEE].properties.base_rental = 800;
            property->house_construction_cost = 2500;
            property->hotel_construction_cost = 10000;
            property->mortgage_val = 4000;
            property->base_price = 8000;
        }
        
        else if (i == BOARD_NUWARA_ELIYA || i == BOARD_GALLE_FACE) {
            property->type = PT_DARK_BLUE;
            board[BOARD_NUWARA_ELIYA].properties.purchase_price = 10000;
            board[BOARD_NUWARA_ELIYA].properties.base_rental = 1000;
            board[BOARD_GALLE_FACE].properties.purchase_price = 12000;
            board[BOARD_GALLE_FACE].properties.base_rental = 1200;
            property->house_construction_cost = 3000;
            property->hotel_construction_cost = 12000;
            property->mortgage_val = 5000;
            property->base_price = 10000;
        }
        property->market_price = property->purchase_price;
        property->current_rent = property->base_rental;
    }
}


void init_railways_utils(struct Square_S board[40]) {
    for (int i = 0; i < 40; i++) {
        if ((board[i].type != ST_RAILWAY) && (board[i].type != ST_UTILITY)) {
            continue;
        }

        board[i].properties.current_owner = PL_NONE;
        board[i].properties.base_rental = 0;
        board[i].properties.is_mortaged  = 0;
        board[i].properties.purchase_price = 1000;  // tmp
    }

}


void init_game_state(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    game_state->current_turn = 1;
    game_state->current_round = 1;
    game_state->current_player = 0;
    game_state->current_inflation_rate = 0;
    game_state->current_interest_rate = 0;
    game_state->bankrupt_count = 0;

    game_state->income_tax_rate = 15;
    game_state->community_dev_fund_tax_rate = 10;

    game_state->active_national_event = NE_NONE;
    game_state->active_regional_card = RD_NONE;
    game_state->active_gov_reg = GR_NONE;

    game_state->market_boom_group = PT_NONE;
    game_state->marker_boom_turns = 0;

    game_state->market_decline_group = PT_NONE;
    game_state->market_decline_turns = 0;

    game_state->gov_reg_rounds = 0;
    game_state->active_economic_event = EE_NONE;
    game_state->economic_event_rounds = 0;
    game_state->current_interest_rate = 8;
    for (int i = 0; i < 8; i++) game_state->group_cooldown[i] = 0;

    game_state->national_event_rounds = 0;
    game_state->national_event_player = PL_NONE;
    game_state->regional_card_rounds = 0;

    reset_ee_mult(monopoly);
    clear_snapshot(&monopoly->game_state.ee_mult.snapshot);
    init_decks(monopoly);

}


void reset_ee_mult(struct Monopoly_S *monopoly) {
    struct EEMultipliers_S *ee_mult = &monopoly->game_state.ee_mult;
    
    ee_mult->hotel_rent_mult = 100;
    ee_mult->south_prop_val_mult = 100;
    
    ee_mult->rail_rent_mult = 100;
    ee_mult->prop_dev_cost_mult = 100;

    ee_mult->insurance_premium_mult = 100;
    ee_mult->coastal_prop_val_mult = 100;

    ee_mult->prop_val_mult = 100;
    ee_mult->rent_mult = 100;
    ee_mult->loan_interest_mult = 100;
    
    ee_mult->house_contruct_cost_mult = 100;
    
    ee_mult->commerce_prop_val = 100;
    
    ee_mult->riot_prob_mult = 100;
    ee_mult->business_interrupt_claim_mult = 100;
}


void clear_snapshot(struct EESnapshot_S *snapshot) {
    memset(snapshot->property_price_change, 0, sizeof(snapshot->property_price_change));
    memset(snapshot->rent_change, 0, sizeof(snapshot->rent_change));
    memset(snapshot->house_construction_cost, 0, sizeof(snapshot->house_construction_cost));
    memset(snapshot->hotel_construction_cost, 0, sizeof(snapshot->hotel_construction_cost));
    snapshot->loan_interrest_rate = 0;
    snapshot->flood_risk = 0;
    snapshot->riot_probability = 0;
    snapshot->business_interruption_claims = 0;
}

int roll_dice() {
    return (rand() % 6) + 1;
}


int first_player(struct Monopoly_S *monopoly) {
    struct Player_S *players = monopoly->players;

    int rolls[4];
    int max_roll = 0;
    int max_idx;

    while (1) {
        for (int i = 0; i < 4; i++) {
            rolls[i] = roll_dice() + roll_dice();
        }
        max_roll = rolls[0];
        max_idx = 0;
        int max_ct = 1;
        for (int i = 1; i < 4; i++) {
            if (rolls[i] == max_roll) {
                max_ct++;
            }
            else if (rolls[i] > max_roll) {
                max_roll = rolls[i];
                max_idx = i;
                max_ct = 1;
            }
        }
        if (max_ct == 1) {
            break;
        } 
    }

    for (int i = 0; i < 4; i++) {
        printf("%s rolls %d.\n", players[i].name, rolls[i]);
    }
    putchar('\n');
    printf("%s will begin the game.\n", players[max_idx].name);
    putchar('\n');
    printf("Turn order :\n");
    for (int i = 0; i < 4; i++) {
        monopoly->game_state.turn_order[i] = (max_idx + i) % 4;
        printf("%s\n", players[(max_idx + i) % 4].name);
    }
    putchar('\n');
    monopoly->game_state.current_turn_order = 0;
    return max_idx;
}


int is_round_passed(struct Monopoly_S *monopoly) {
    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[i];
        if (player->is_bankrupt) continue;
        if (player->passed_go == 0) return 0;
    }
    return 1;
}


void before_roll(struct Monopoly_S *monopoly) {
    maintain_buildings(monopoly);
}


void player_land(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *landed_sq = &monopoly->board[player->current_pos];
    printf("%s landed on %s\n", player->name, landed_sq->name);

    switch (landed_sq->type) {
        case ST_START:
            break;

        case ST_PROPERTY:
            land_property(monopoly);
            break;
            
        case ST_RAILWAY:
            land_railway(monopoly);
            break;

        case ST_EVENT:
            land_event(monopoly);
            break;

        case ST_TAX:
            land_tax(monopoly);
            break;

        case ST_INSURANCE:
            land_insurance(monopoly);
            break;
        
        case ST_SPECIAL:
            land_special(monopoly);
            break;

        case ST_UTILITY:
            land_utility(monopoly);
            break;

        case ST_BANK:
            land_bank(monopoly);
            break;
    }
}




void land_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *prop = &sq->properties;
    //buy /auction

    int dec = eval_propert_buy(monopoly);
    if (dec == 1) {
        player_buy_property(monopoly);
        return;
    }
    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if ((prop->current_owner != player->id) && (prop->current_owner != PL_NONE)) {
        player_property_pay_rent(monopoly);
    }

    if (prop->current_owner == player->id) {
        // dec = eval_develop_property(monopoly);
        // if (dec == 1) player_develop_house(monopoly);
        // else if (dec == 2) player_develop_hotel(monopoly);

        
        dec = eval_renovate_property(monopoly);
        if (dec == 1) player_renovate_property(monopoly);
        return;

    }
}


void land_railway(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *rail = &monopoly->board[player->current_pos].properties;

    int dec = eval_railway_buy(monopoly);
    if (dec == 1) {
        player_buy_util_railway(monopoly);
        return;
    }

    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if  ((rail->current_owner != player->id) && (rail->current_owner != PL_NONE)) {
        player_railway_pay_rent(monopoly);
    }
}


void land_event(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];

    switch(sq->id) {
        case BOARD_COMMUNITY_FUND:
            player_pay_community_fund(monopoly);
            break;

        case BOARD_NATIONAL_EVENT_1:
        case BOARD_NATIONAL_EVENT_2:
        case BOARD_NATIONAL_EVENT_3:
            draw_national_event(monopoly);
            break;
    }
}


void land_tax(struct Monopoly_S *monopoly) {
    player_pay_taxes(monopoly);
}


void land_insurance(struct Monopoly_S *monopoly) {
    player_buy_insurance(monopoly);
}


void land_special(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    switch (sq->id) {
        case BOARD_GO_TO_JAIL:
            player_go_to_jail(monopoly);
            break;
    }
}


void land_utility(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *util = &monopoly->board[player->current_pos].properties;

    int dec = eval_util_buy(monopoly);
    if (dec == 1) {
        player_buy_util_railway(monopoly);
        return;
    }

    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if  ((util->current_owner != player->id) && (util->current_owner != PL_NONE)) {
        player_railway_pay_rent(monopoly);
    }
}


void land_bank(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan == 0) {
        player_take_loan(monopoly);
        return;
    }

    int outstanding = player->loan_amount + player->accrued_interest;
    int dec = eval_repay_loan(monopoly);

    if (dec == 1) player_repay_loan(monopoly, outstanding / 4);
    else if (dec == 2) player_repay_loan(monopoly, outstanding);
    else if (dec == 3) player_extend_loan(monopoly);
}


int auction(struct Monopoly_S *monopoly, struct Square_S *sq) {
    printf("Auction Started.\n");
    printf("Property :\n%s\n", sq->name);
    printf("Opening Bid :\n%d\n", sq->properties.purchase_price / 2);

    int bid = sq->properties.purchase_price / 2 - 250;
    int max_idx = -1;
    int withdrawed[4] = {0};
    int withdraw_count = 0;
    int i = monopoly->game_state.current_turn_order + 1 % 4;
    struct Player_S *player = &monopoly->players[monopoly->game_state.turn_order[i]];

    int player_count = 4 - monopoly->game_state.bankrupt_count;
    while (1) {

        if (withdraw_count == player_count - 1) break;

        if (player->is_bankrupt) {
            player = &monopoly->players[monopoly->game_state.turn_order[++i % 4]];
            if (withdrawed[player->id] == 0) {
                withdraw_count++;
                withdrawed[player->id] = 1;
            }
            continue;
        }

        if (withdrawed[player->id] == 1) {
            player = &monopoly->players[monopoly->game_state.turn_order[++i % 4]];
            continue;
        }

        int dec = eval_auction_bid(player, &sq->properties, bid);
        if (dec == 1) {
            bid += 250;
            printf("%s bids LKD %d.\n", player->name, bid);
            max_idx = player->id;
        }
        else {
            printf("%s withdraws.\n", player->name);
            withdraw_count++;
            withdrawed[player->id] = 1;
        }

        player = &monopoly->players[monopoly->game_state.turn_order[++i % 4]];
    }

    if (max_idx == -1) {
        printf("No bids.\n");
        return 0;
    }

    if (sq->properties.current_owner != PL_NONE) {
        struct Player_S *owner = &monopoly->players[sq->properties.current_owner];
        if (owner->property_owned > 0) owner->property_owned--;
    }

    struct Player_S *winner = &monopoly->players[max_idx];
    printf("%s wins the auction.\n", winner->name);
    winner->cash -= bid;
    sq->properties.current_owner = max_idx;
    switch (sq->type) {
        case ST_PROPERTY : winner->property_owned++; break;
        case ST_RAILWAY  : winner->railway_owned++;  break;
        case ST_UTILITY  : winner->util_owned++;     break;
        default: break;
    }
    return bid;
}


int is_monopoly(struct Monopoly_S *monopoly, struct Player_S *player, enum PropertyType_E p_type) {
    struct Square_S *board =  monopoly->board;

    for (int i = 0; i < 40; i++) {
        if (board[i].type != ST_PROPERTY) continue;
        if (board[i].properties.type != p_type) continue;

        if (board[i].properties.current_owner != player->id) return 0;
    }
    return 1;
}
    

void round_summary(struct Monopoly_S *monopoly) {
    printf("\n=============================================\n");
    printf("Round %d Summary\n", monopoly->game_state.current_round);
    printf("=============================================\n\n");
    

    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[monopoly->game_state.turn_order[i]];

        printf("%s\n", player->name);
        printf("Cash : LKR %d.\n", player->cash);
        printf("Net Worth : LKR %d\n", player->net_worth);
        printf("Properties : %d\n", player->property_owned);
        printf("Hotels : %d\n", player->hotels_owned);
        printf("Outstanding Loan : LKR %d.\n", player->loan_amount);
        if (i != 3) printf("\n---------------------------------------------\n");
    }
    printf("=============================================\n\n");
}


void end_game_summary(struct Monopoly_S *monopoly) {
    struct Player_S *player = NULL;

    if (monopoly->game_state.bankrupt_count == 3) {
        for (int i = 0; i < 4; i++) {
            struct Player_S *player_tmp = &monopoly->players[i];
            if (player_tmp->is_bankrupt) continue;
            player = player_tmp;
        }
    }

    else {
        int max_networth = monopoly->players[0].net_worth;
        int max_net_id = 0;
        for (int i = 1; i < 4; i++) {
            struct Player_S *player_tmp = &monopoly->players[i];
            if (player_tmp->net_worth > max_networth) {
                max_networth = player_tmp->net_worth;
                max_net_id = i;
            }
        }
        player = &monopoly->players[max_net_id];
    }

    printf("\n=============================================\n\n");   
    printf("GAME OVER\n\n");

    printf("Winner\n");
    printf("%s\n\n", player->name);

    printf("Total Cash\n");
    printf("%d\n\n", player->cash);

    printf("Outstanding Loans\n");
    if (player->has_active_loan) {
        printf("%d\n\n", player->loan_amount);
    }
    else {
        printf("None\n\n");
    }

    calc_networth(monopoly);
    printf("Networth\n");
    printf("%d\n\n", player->net_worth);

    printf("=============================================\n\n");
}   


void calc_networth(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;
    
    int net_worth = 0;

    int prop_val = 0;
    int building_val = 0;
    int rail_val = 0;
    int util_val = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type == ST_PROPERTY) {
            if (sq->properties.current_owner == player->id) {
                prop_val += property_market_value(monopoly, i);
                building_val += sq->properties.house_count * sq->properties.house_construction_cost;
                building_val += sq->properties.hotel_count * sq->properties.hotel_construction_cost;
            }
        }
    }

    rail_val = player->railway_owned * 1000;
    util_val = player->util_owned * 1000;
    
    player->property_val = prop_val;
    player->building_val = building_val;
    player->railway_val = rail_val;
    player->utilitty_val = util_val;

    player->insurance_claim_recivable = 0;
    player->accrued_interest = 0;
    player->taxes_due = 0;

    net_worth = player->cash + player->property_val + player->building_val + player->railway_val + player->utilitty_val 
                + player->insurance_claim_recivable - player->loan_amount - player->accrued_interest - player->taxes_due;

    player->net_worth = net_worth;
}


void check_bankrupt(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    if (player->is_bankrupt) return;

    if (player->net_worth < 0) {
        eliminate_player(monopoly, player);
    }
}


void prop_building_condition(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    int luxury_tax = (monopoly->game_state.active_gov_reg == GR_LUXURY_PROPERTY_TAX);

    // prop/ building depreciation
    for (int i = 0; i < 40; i++) {
        if (board[i].type == ST_PROPERTY) {
            struct Square_S *sq = &board[i];
            struct Property_S *property = &sq->properties;

            property->property_age++;
            if (property->property_age > 50 && property->depreciation_precent < 30) property->depreciation_precent++;
            // printf("%s - %d - ", sq->name, property->depreciation_precent);

            if (property->hotel_count) {
                property->hotel_condition -= 2;
                if (property->hotel_condition < 0) property->hotel_condition = 0;
                // printf("hotel : %d - ",  property->hotel_condition);

                if (luxury_tax && property->current_owner != PL_NONE) {
                    struct Player_S *owner = &monopoly->players[property->current_owner];
                    int tax = (property->market_price + property->hotel_construction_cost) * 25 / 100;
                    owner->cash -= tax;
                    printf("%s paid LKR %d luxury property tax on %s.\n", owner->name, tax, sq->name);
                }
            }

            // printf("house : ");
            for (int j = 0;j < property->house_count; j++) {
                property->house_condition[j] -= 2;
                if (property->house_condition[j] < 0) property->house_condition[j] = 0;
                // printf("%d, ",  property->house_condition[j]);

            }
            // printf("\n");

            if (property->house_count || property->hotel_count) {
                int worst = property->hotel_count ? property->hotel_condition : property->house_condition[0];
                if (worst < 100) property->missed_maintenance++;
                else property->missed_maintenance = 0;
            }

            if (property->missed_maintenance > 20 && property->is_damaged == 0) {
                if (property->house_count || property->hotel_count) {
                    property->is_damaged = 1;
                    property->market_price -= property->market_price * 15 / 100;
                    property->current_rent -= property->current_rent * 25 / 100;
                    property->maintenance_cost_mult = 150;
                    property->repair_cost = property->purchase_price / 4;
                    property->missed_maintenance = 0;

                    printf("Structural damage on %s.\n", sq->name);
                }
            }

        }
    }
}


void apply_inflation(struct Monopoly_S *monopoly) {
    int round = monopoly->game_state.current_round;
    if (round % 10 == 0) {
        int inflation_dict[] = {-3, 0, 2, 5, 8, 12};

        int inflation = inflation_dict[rand() % 6];

        monopoly->game_state.current_inflation_rate = inflation;
    }

    int inflation = monopoly->game_state.current_inflation_rate;
    struct Square_S *board = monopoly->board;
    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        switch (sq->type) {
            case ST_PROPERTY:
                struct Property_S *prop = &sq->properties;
                prop->market_price += prop->market_price * inflation / 100;
                prop->house_construction_cost += prop->house_construction_cost * inflation / 100;
                prop->hotel_construction_cost += prop->hotel_construction_cost * inflation / 100;
                prop->current_rent += prop->current_rent * inflation / 100;
                break;

            case ST_RAILWAY:
                struct Property_S *rail = &sq->properties;
                rail->current_rent += prop->current_rent * inflation / 100;
                break;

            case ST_UTILITY:
                struct Property_S *util = &sq->properties;
                util->current_rent += util->current_rent * inflation / 100;
                break;

            default:
                break;
        }
    }
}
 

void develope_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    if (monopoly->game_state.ne_mult.construction_suspended) return;

    int dec = 0;
    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        
        dec = eval_develop_property(monopoly, property);
        if (dec == 1) player_develop_house(monopoly, sq);
        else if (dec == 2) player_develop_hotel(monopoly, sq);
    }
}


void draw_economic_event(struct Monopoly_S *monopoly) {
    if (monopoly->game_state.active_economic_event >= 0) monopoly->game_state.economic_event_rounds--;

    if (monopoly->game_state.current_round % 15 != 0) return;

    struct EEMultipliers_S *ee_mult = &monopoly->game_state.ee_mult;
    reset_ee_mult(monopoly);
    
    int ee_index = rand() % 8;
    monopoly->game_state.active_economic_event = ee_index;
    monopoly->game_state.economic_event_rounds = 15;
    
    clear_snapshot(&ee_mult->snapshot);
    
    printf("Economic Event\n");
    switch (ee_index) {
    case EE_TOURISM_BOOM:
        ee_mult->hotel_rent_mult = 200;
        ee_mult->south_prop_val_mult = 115;

        printf("Tourism Boom\n");
        printf("Hotels receive double rent.\n");    // done
        printf("Southern coastal properties increase in value by 15%%.\n");
        break;
    
    case EE_FUEL_CRISIS:
        ee_mult->rail_rent_mult = 200;  // done
        ee_mult->prop_dev_cost_mult = 120;  // done

        printf("Fuel Crisis\n");
        printf("Railway rent doubles.\n");  
        printf("Property development costs increase by 20%%.\n");
        break;
    
    case EE_HEAVY_MONSOON:
        ee_mult->insurance_premium_mult = 120;  // tmp
        ee_mult->coastal_prop_val_mult = 190;

        printf("Heavy Monsoon\n");
        printf("Flood risk increases.\n");
        printf("Insurance premiums increase.\n");
        printf("Coastal properties lose 10%% value.\n");
        break;
    
    case EE_ECONOMIC_RECESSION:
        ee_mult->prop_val_mult = 85;
        ee_mult->rent_mult = 100;
        ee_mult->loan_interest_mult = 115;

        printf("Economic Recession\n");
        printf("Property values decrease by 15%%.\n");
        printf("Rent decreases by 10%%.\n");
        printf("Loan interest increases by 15%%.\n");
        break;
    
    case EE_STOCK_MARKET_BOOM:
        ee_mult->prop_val_mult = 110;
        ee_mult->loan_interest_mult = 90;

        printf("Stock Market Boom\n");
        printf("Property values increase by 10%%.\n");
        printf("Loan interest decreases by 10%%.\n");
        break;
    
    case EE_GOVERNMENT_HOUSING_PROGRAMME:
        ee_mult->house_contruct_cost_mult = 75; // done

        printf("Government Housing Programme\n");
        printf("House construction costs reduce by 25%%.\n");
        break;
    
    case EE_FOREIGN_INVESTMENT:
        ee_mult->commerce_prop_val = 20;    // done

        printf("Foreign Investment\n");
        printf("Commercial properties increase by 20%%.\n");
        break;
    
    case EE_POLITICAL_UNREST:
        ee_mult->riot_prob_mult = 200;
        ee_mult->hotel_rent_mult = 50;
        ee_mult->business_interrupt_claim_mult = 125;   // tmp

        printf("Political Unrest\n");
        printf("Riot probability doubles.\n");
        break;
    
    default:
        printf("No active economic event.\n");
        break;
    }

    // update snapshot
}

void apply_economic_event(struct Monopoly_S *monopoly) {
    // alter game state while updating the snapshot
    struct EEMultipliers_S *mult = &monopoly->game_state.ee_mult;
    struct EESnapshot_S *snap = &mult->snapshot;

    // property values
    struct Square_S *board = monopoly->board;
    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type == ST_PROPERTY) {
            struct Property_S *property = &sq->properties;

            int old_price = property->market_price;
            int old_rent = property->current_rent;

            property->market_price = property->market_price * mult->prop_val_mult / 100;
            property->current_rent = property->current_rent * mult->rent_mult / 100;

            if (is_south_coastal(i)) property->market_price = property->market_price * mult->south_prop_val_mult / 100;
            if (is_coastal(i)) property->market_price = property->market_price * mult->coastal_prop_val_mult / 100;
            if (is_commercial(i)) property->market_price = property->market_price * mult->commerce_prop_val / 100;

            snap->property_price_change[i] = property->market_price - old_price;
            snap->rent_change[i] = property->current_rent - old_rent;

            int old_house = property->house_construction_cost;
            int old_hotel = property->hotel_construction_cost;

            property->house_construction_cost = property->house_construction_cost * mult->house_contruct_cost_mult / 100;
            property->house_construction_cost = property->house_construction_cost * mult->prop_dev_cost_mult / 100;
            property->hotel_construction_cost = property->hotel_construction_cost * mult->prop_dev_cost_mult / 100;

            snap->house_construction_cost[i] = property->house_construction_cost - old_house;
            snap->hotel_construction_cost[i] = property->hotel_construction_cost - old_hotel;
        }
    }

    snap->loan_interrest_rate = monopoly->game_state.current_interest_rate * mult->loan_interest_mult / 100
                                - monopoly->game_state.current_interest_rate;
    snap->riot_probability = mult->riot_prob_mult - 100;
    snap->business_interruption_claims = mult->business_interrupt_claim_mult - 100;
}
