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

#define ROUNDS 500


/*
runs turns and rounds until one player is left or 500 rounds pass
*/
void game_loop(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    first_player(monopoly);

    while (game_state->current_round <= ROUNDS) {
        printf("\n----------Turn: %d----------\n", game_state->current_turn);

        for (int i = 0; i < 4; i++) {
            if (game_state->bankrupt_count >= 3) {
                round_summary(monopoly);
                end_game_summary(monopoly);
                return;
            }

            game_state->current_player = game_state->turn_order[game_state->current_turn_order];
            struct Player_S *player = &monopoly->players[game_state->current_player];

            if (player->is_bankrupt) {
                game_state->current_turn_order = (game_state->current_turn_order + 1) % 4;
                game_state->current_turn++;
                continue;
            }

            before_roll(monopoly);

            putchar('\n');
            player_move(monopoly);
            player_land(monopoly);
            develope_property(monopoly);
            calc_networth(monopoly);
            check_bankrupt(monopoly);

            game_state->current_turn++;
            game_state->current_turn_order = (game_state->current_turn_order + 1) % 4;
        }

        if (is_round_passed(monopoly) != 0) {
            prop_building_condition(monopoly);
            apply_inflation(monopoly);
            accrue_loan_interest(monopoly);
            insurance_tick(monopoly);
            luxury_property_tax(monopoly);

            draw_economic_event(monopoly);
            if (game_state->current_round % 10 == 0) {
                market_review(monopoly);
                random_disaster(monopoly);
            }
            if (game_state->current_round % 15 == 0) draw_regional_card(monopoly);
            if (game_state->current_round % 20 == 0) draw_gov_regulation(monopoly);

            expire_events(monopoly);

            round_summary(monopoly);
            market_conditions(monopoly);

            game_state->current_round++;
            for (int i = 0; i < 4; i++) {
                monopoly->players[i].passed_go--;
            }
        }
    }

    end_game_summary(monopoly);
}


/*
prints the opening banner
*/
void start_msg(struct Player_S players[4]) {
    printf("MONOPOLY-LK Simulation\n\n");
    for (int i = 0; i < 4; i++) {
        printf("Player %d : %s.\n", players[i].id + 1, players[i].name);
    }
    putchar('\n');
    printf("Each player begins with LKR 30,000.\n\n");
}


/*
creates the four strategy players
*/
void init_players_all(struct Player_S players[4]) {
    player_init(&players[0], 0, PL_AGGRESIVE_INVESTOR, "Aggresive Investor");
    player_init(&players[1], 1, PL_CONSERVATIVE_BANKER, "Conservative Banker");
    player_init(&players[2], 2, PL_RISK_TAKER, "Risk Taker");
    player_init(&players[3], 3, PL_OPPORTUNISTIC_TRADER, "Opportunistic Trader");
}


/*
sets the starting game state and shuffles the decks
*/
void init_game_state(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    game_state->current_turn = 1;
    game_state->current_round = 1;
    game_state->current_player = 0;
    game_state->current_inflation_rate = 0;
    game_state->current_interest_rate = 8;
    game_state->bankrupt_count = 0;

    game_state->income_tax_rate = 15;
    game_state->community_dev_fund_tax_rate = 10;

    game_state->active_national_event = NE_NONE;
    game_state->national_event_rounds = 0;
    game_state->national_event_player = PL_NONE;

    game_state->active_regional_card = RD_NONE;
    game_state->regional_card_rounds = 0;

    game_state->active_gov_reg = GR_NONE;
    game_state->gov_reg_rounds = 0;

    game_state->active_economic_event = EE_NONE;
    game_state->economic_event_rounds = 0;

    game_state->market_boom_group = PT_NONE;
    game_state->marker_boom_turns = 0;

    game_state->market_decline_group = PT_NONE;
    game_state->market_decline_turns = 0;

    for (int i = 0; i < 8; i++) game_state->group_cooldown[i] = 0;

    reset_ee_mult(monopoly);
    reset_ne_mult(monopoly);
    reset_rd_mult(monopoly);
    init_decks(monopoly);
}


/*
one six sided die
*/
int roll_dice() {
    return (rand() % 6) + 1;
}


/*
rolls for turn order and returns the first player
*/
int first_player(struct Monopoly_S *monopoly) {
    struct Player_S *players = monopoly->players;

    int rolls[4];
    int max_roll = 0;
    int max_idx = 0;

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
        if (max_ct == 1) break;
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


/*
1 once every solvent player has passed GO
*/
int is_round_passed(struct Monopoly_S *monopoly) {
    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[i];
        if (player->is_bankrupt) continue;
        if (player->passed_go == 0) return 0;
    }
    return 1;
}


/*
maintenance and repairs before the dice are rolled
*/
void before_roll(struct Monopoly_S *monopoly) {
    maintain_buildings(monopoly);
}


/*
resolves whatever square the current player landed on
*/
void player_land(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *landed_sq = &monopoly->board[player->current_pos];

    printf("%s landed on %s.\n", player->name, landed_sq->name);

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

    int dec = eval_propert_buy(monopoly);
    if (dec == 1) {
        player_buy_property(monopoly);
        return;
    }
    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if (prop->current_owner != player->id && prop->current_owner != PL_NONE) {
        player_property_pay_rent(monopoly);
        return;
    }

    if (prop->current_owner == player->id) {
        if (eval_renovate_property(monopoly) == 1) player_renovate_property(monopoly);
    }
}


void land_railway(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];
    struct Property_S *rail = &sq->properties;

    int dec = eval_railway_buy(monopoly);
    if (dec == 1) {
        player_buy_util_railway(monopoly);
        return;
    }
    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if (rail->current_owner != player->id && rail->current_owner != PL_NONE) {
        player_railway_pay_rent(monopoly);
    }
}


void land_event(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *sq = &monopoly->board[player->current_pos];

    switch (sq->id) {
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
    struct Property_S *util = &sq->properties;

    int dec = eval_util_buy(monopoly);
    if (dec == 1) {
        player_buy_util_railway(monopoly);
        return;
    }
    else if (dec == -1) {
        auction(monopoly, sq);
        return;
    }

    if (util->current_owner != player->id && util->current_owner != PL_NONE) {
        player_util_pay_rent(monopoly);
    }
}


void land_bank(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->has_active_loan == 0) {
        player_take_loan(monopoly);
        return;
    }

    int outstanding = player->loan_amount + player->accrued_interest;

    switch (eval_repay_loan(monopoly)) {
        case 1: player_repay_loan(monopoly, outstanding / 4); break;
        case 2: player_repay_loan(monopoly, outstanding);     break;
        case 3: player_extend_loan(monopoly);                 break;
        case 4: player_increase_loan(monopoly);               break;
    }
}


/*
runs an auction and returns the winning bid
*/
int auction(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct GameState_S *game_state = &monopoly->game_state;

    if (sq->properties.is_loan_locked) return 0;

    int opening = property_value(monopoly, sq) / 2;
    if (sq->properties.type == game_state->market_decline_group) opening = opening * 75 / 100;

    printf("Auction Started.\n");
    printf("Property :\n%s\n", sq->name);
    printf("Opening Bid :\nLKR %d.\n", opening);

    int bid = opening - 250;
    int max_idx = -1;
    int withdrawed[4] = {0};
    int withdraw_count = 0;
    int i = (game_state->current_turn_order + 1) % 4;
    struct Player_S *player = &monopoly->players[game_state->turn_order[i]];

    int player_count = 4 - game_state->bankrupt_count;

    while (1) {
        if (withdraw_count >= player_count - 1) break;

        if (player->is_bankrupt || withdrawed[player->id]) {
            if (player->is_bankrupt && withdrawed[player->id] == 0) {
                withdrawed[player->id] = 1;
                withdraw_count++;
            }
            player = &monopoly->players[game_state->turn_order[++i % 4]];
            continue;
        }

        if (eval_auction_bid(monopoly, player, sq, bid) == 1) {
            bid += 250;
            max_idx = player->id;
            printf("%s bids LKR %d.\n", player->name, bid);
        }
        else {
            withdrawed[player->id] = 1;
            withdraw_count++;
            printf("%s withdraws.\n", player->name);
        }

        player = &monopoly->players[game_state->turn_order[++i % 4]];
    }

    if (max_idx == -1) {
        printf("No bids. Ownership remains with the Bank.\n");
        return 0;
    }

    if (sq->properties.current_owner != PL_NONE) {
        struct Player_S *owner = &monopoly->players[sq->properties.current_owner];
        switch (sq->type) {
            case ST_PROPERTY: if (owner->property_owned > 0) owner->property_owned--; break;
            case ST_RAILWAY:  if (owner->railway_owned > 0)  owner->railway_owned--;  break;
            case ST_UTILITY:  if (owner->util_owned > 0)     owner->util_owned--;     break;
            default: break;
        }
    }

    struct Player_S *winner = &monopoly->players[max_idx];
    winner->cash -= bid;
    sq->properties.current_owner = max_idx;

    switch (sq->type) {
        case ST_PROPERTY: winner->property_owned++; break;
        case ST_RAILWAY:  winner->railway_owned++;  break;
        case ST_UTILITY:  winner->util_owned++;     break;
        default: break;
    }

    printf("%s wins the auction.\n", winner->name);
    return bid;
}


/*
1 if the player owns every property in a colour group
*/
int is_monopoly(struct Monopoly_S *monopoly, struct Player_S *player, enum PropertyType_E p_type) {
    struct Square_S *board = monopoly->board;

    for (int i = 0; i < 40; i++) {
        if (board[i].type != ST_PROPERTY) continue;
        if (board[i].properties.type != p_type) continue;
        if (board[i].properties.current_owner != player->id) return 0;
    }
    return 1;
}


/*
builds houses and hotels across everything the current player owns
*/
void develope_property(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];
    struct Square_S *board = monopoly->board;

    if (monopoly->game_state.ne_mult.construction_suspended) return;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner != player->id) continue;
        if (property->is_mortaged) continue;

        int dec = eval_develop_property(monopoly, property);
        if (dec == 1) player_develop_house(monopoly, sq);
        else if (dec == 2) player_develop_hotel(monopoly, sq);
    }
}


/*
eliminates the current player when net worth goes negative
*/
void check_bankrupt(struct Monopoly_S *monopoly) {
    struct Player_S *player = &monopoly->players[monopoly->game_state.current_player];

    if (player->is_bankrupt) return;
    if (player->net_worth < 0) eliminate_player(monopoly, player);
}


/*
prints every player standing at the end of a round
*/
void round_summary(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    int saved = game_state->current_player;

    printf("\n=============================================\n");
    printf("Round %d Summary\n", game_state->current_round);
    printf("=============================================\n\n");

    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[game_state->turn_order[i]];

        game_state->current_player = player->id;
        calc_networth(monopoly);

        printf("%s\n", player->name);
        printf("Cash : LKR %d\n", player->cash);
        printf("Net Worth : LKR %d\n", player->net_worth);
        printf("Properties : %d\n", player->property_owned);
        printf("Hotels : %d\n", player->hotels_owned);
        printf("Outstanding Loan : LKR %d\n", player->loan_amount + player->accrued_interest);
        if (i != 3) printf("\n---------------------------------------------\n");
    }

    game_state->current_player = saved;
    printf("=============================================\n\n");
}


/*
prints the winner and their final position
*/
void end_game_summary(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Player_S *winner = NULL;

    int saved = game_state->current_player;
    for (int i = 0; i < 4; i++) {
        game_state->current_player = i;
        calc_networth(monopoly);
    }
    game_state->current_player = saved;

    for (int i = 0; i < 4; i++) {
        struct Player_S *player = &monopoly->players[i];
        if (player->is_bankrupt) continue;
        if (winner == NULL || player->net_worth > winner->net_worth) winner = player;
    }

    if (winner == NULL) winner = &monopoly->players[0];

    printf("\n=============================================\n\n");
    printf("GAME OVER\n\n");

    printf("Winner\n");
    printf("%s\n\n", winner->name);

    printf("Total Cash\n");
    printf("LKR %d\n\n", winner->cash);

    printf("Total Property Value\n");
    printf("LKR %d\n\n", winner->property_val + winner->building_val + winner->railway_val + winner->utilitty_val);

    printf("Outstanding Loans\n");
    if (winner->has_active_loan) printf("LKR %d\n\n", winner->loan_amount + winner->accrued_interest);
    else printf("None\n\n");

    printf("Net Worth\n");
    printf("LKR %d\n\n", winner->net_worth);

    printf("=============================================\n\n");
}
