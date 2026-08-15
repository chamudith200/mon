#include "events.h"
#include "types.h"
#include "game.h"
#include "player.h"

#include <stdio.h>
#include <stdlib.h>

#define NE_DECK_SIZE 20
#define RD_DECK_SIZE 12


/*
builds and shuffles both card decks before the game starts
*/
void init_decks(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    deck_init(&game_state->national_deck, NE_DECK_SIZE);
    deck_init(&game_state->regional_deck, RD_DECK_SIZE);

    deck_shuffle(&game_state->national_deck);
    deck_shuffle(&game_state->regional_deck);
}


/*
fills a deck with card indexes 0 to count - 1
*/
void deck_init(struct Deck_S *deck, int count) {
    deck->count = count;
    for (int i = 0; i < count; i++) {
        deck->cards[i] = i;
    }
}


/*
fisher yates shuffle over the whole deck
*/
void deck_shuffle(struct Deck_S *deck) {
    for (int i = deck->count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = tmp;
    }
}


/*
takes the top card and sends it to the back of the deck
*/
int deck_draw(struct Deck_S *deck) {
    int card = deck->cards[0];

    for (int i = 0; i < deck->count - 1; i++) {
        deck->cards[i] = deck->cards[i + 1];
    }
    deck->cards[deck->count - 1] = card;

    return card;
}


/*
sets every economic event multiplier back to 100
*/
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


/*
sets every national event multiplier back to 100
*/
void reset_ne_mult(struct Monopoly_S *monopoly) {
    struct NEMultipliers_S *ne_mult = &monopoly->game_state.ne_mult;

    ne_mult->hotel_rent_mult = 100;
    ne_mult->rail_rent_mult = 100;
    ne_mult->util_income_mult = 100;

    ne_mult->prop_val_mult = 100;
    ne_mult->commerce_prop_val_mult = 100;
    ne_mult->rail_val_mult = 100;
    ne_mult->revalued_group = PT_NONE;
    ne_mult->revalued_group_mult = 100;

    ne_mult->house_construct_cost_mult = 100;
    ne_mult->construct_cost_mult = 100;
    ne_mult->insurance_premium_mult = 100;

    ne_mult->loan_interest_change = 0;
    ne_mult->construction_suspended = 0;
}


/*
sets every regional development multiplier back to 100
*/
void reset_rd_mult(struct Monopoly_S *monopoly) {
    struct RDMultipliers_S *rd_mult = &monopoly->game_state.rd_mult;

    rd_mult->south_rent_mult = 100;
    rd_mult->airport_rent_mult = 100;

    rd_mult->colombo_val_mult = 100;
    rd_mult->it_zone_val_mult = 100;
    rd_mult->north_val_mult = 100;
    rd_mult->hill_val_mult = 100;
    rd_mult->kandy_val_mult = 100;
    rd_mult->coastal_val_mult = 100;
    rd_mult->water_area_val_mult = 100;

    rd_mult->rail_rent_mult = 100;
    rd_mult->util_rent_mult = 100;
}


/*
picks a random economic event every 15 rounds
*/
void draw_economic_event(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct EEMultipliers_S *ee_mult = &game_state->ee_mult;

    if (game_state->current_round % 15 != 0) return;

    int event = rand() % 8;

    reset_ee_mult(monopoly);
    game_state->active_economic_event = event;
    game_state->economic_event_rounds = 15;

    printf("Economic Event\n");
    printf("%s\n", economic_event_name(event));

    switch (event) {
    case EE_TOURISM_BOOM:
        ee_mult->hotel_rent_mult = 200;
        ee_mult->south_prop_val_mult = 115;

        printf("Hotels receive double rent.\n");
        printf("Southern coastal properties increase in value by 15%%.\n");
        break;

    case EE_FUEL_CRISIS:
        ee_mult->rail_rent_mult = 200;
        ee_mult->prop_dev_cost_mult = 120;

        printf("Railway rent doubles.\n");
        printf("Property development costs increase by 20%%.\n");
        break;

    case EE_HEAVY_MONSOON:
        ee_mult->insurance_premium_mult = 120;
        ee_mult->coastal_prop_val_mult = 90;

        printf("Flood risk increases.\n");
        printf("Insurance premiums increase.\n");
        printf("Coastal properties lose 10%% value.\n");
        break;

    case EE_ECONOMIC_RECESSION:
        ee_mult->prop_val_mult = 85;
        ee_mult->rent_mult = 90;
        ee_mult->loan_interest_mult = 115;

        printf("Property values decrease by 15%%.\n");
        printf("Rent decreases by 10%%.\n");
        printf("Loan interest increases by 15%%.\n");
        break;

    case EE_STOCK_MARKET_BOOM:
        ee_mult->prop_val_mult = 110;
        ee_mult->loan_interest_mult = 90;

        printf("Property values increase by 10%%.\n");
        printf("Loan interest decreases by 10%%.\n");
        break;

    case EE_GOVERNMENT_HOUSING_PROGRAMME:
        ee_mult->house_contruct_cost_mult = 75;

        printf("House construction costs reduce by 25%%.\n");
        break;

    case EE_FOREIGN_INVESTMENT:
        ee_mult->commerce_prop_val = 120;

        printf("Commercial properties increase by 20%%.\n");
        break;

    case EE_POLITICAL_UNREST:
        ee_mult->riot_prob_mult = 200;
        ee_mult->hotel_rent_mult = 50;
        ee_mult->business_interrupt_claim_mult = 125;

        printf("Riot probability doubles.\n");
        printf("Hotel rent drops by 50%%.\n");
        break;

    default:
        break;
    }
    putchar('\n');
}


/*
draws the top national event card for the current player
*/
void draw_national_event(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct NEMultipliers_S *ne_mult = &game_state->ne_mult;
    struct Player_S *player = &monopoly->players[game_state->current_player];

    int card = deck_draw(&game_state->national_deck);
    struct Square_S *target = NULL;
    int idx = 0;

    reset_ne_mult(monopoly);
    game_state->active_national_event = card;
    game_state->national_event_player = player->id;
    game_state->national_event_rounds = 15;

    printf("National Event Card\n");
    printf("%s\n", national_event_name(card));

    switch (card) {
    case NE_TOURISM_HYPE:
        ne_mult->hotel_rent_mult = 200;
        game_state->national_event_rounds = 5;

        printf("Hotels earn double rent for 5 rounds.\n");
        break;

    case NE_FUEL_SHORTAGE:
        ne_mult->rail_rent_mult = 200;
        game_state->national_event_rounds = 5;

        printf("Railway rent doubles for 5 rounds.\n");
        break;

    case NE_HEAVY_FLOODS:
        target = random_coastal_property(monopoly);
        if (target == NULL) {
            printf("No coastal property was affected.\n");
            break;
        }
        damage_property(monopoly, target);

        printf("Affected Property :\n%s\n", target->name);
        break;

    case NE_POLITICAL_RALLY:
        target = random_owned_property(monopoly, 0);
        if (target == NULL) {
            printf("No property was closed.\n");
            break;
        }
        target->properties.closed_rounds = 2;

        printf("%s is closed for 2 rounds.\n", target->name);
        break;

    case NE_STOCK_MARKET_RISE:
        ne_mult->prop_val_mult = 110;

        printf("All property values increase by 10%%.\n");
        break;

    case NE_ECONOMIC_DOWNTURN:
        ne_mult->prop_val_mult = 85;

        printf("Property values decrease by 15%%.\n");
        break;

    case NE_HOUSING_SUBSIDY:
        ne_mult->house_construct_cost_mult = 70;

        printf("House construction cost reduced by 30%%.\n");
        break;

    case NE_INTEREST_RATE_CUT:
        ne_mult->loan_interest_change = -2;

        printf("Loan interest reduced by 2%%.\n");
        break;

    case NE_INTEREST_RATE_INCREASE:
        ne_mult->loan_interest_change = 2;

        printf("Loan interest increased by 2%%.\n");
        break;

    case NE_TAX_AMNESTY:
        for (int i = 0; i < 4; i++) {
            if (monopoly->players[i].is_bankrupt) continue;
            monopoly->players[i].cash += 2000;
        }

        printf("Each player receives LKR 2000.\n");
        break;

    case NE_POWER_FAILURE:
        ne_mult->util_income_mult = 50;
        game_state->national_event_rounds = 3;

        printf("Utility income halved for 3 rounds.\n");
        break;

    case NE_FOREIGN_FUNDING:
        ne_mult->commerce_prop_val_mult = 115;

        printf("Commercial property values increase by 15%%.\n");
        break;

    case NE_PORT_EXPANSION:
        ne_mult->rail_val_mult = 120;

        printf("Railway station values increase by 20%%.\n");
        break;

    case NE_FESTIVAL_SEASON:
        ne_mult->hotel_rent_mult = 150;

        printf("Hotels receive 50%% additional rent.\n");
        break;

    case NE_LABOUR_STRIKE:
        ne_mult->construction_suspended = 1;
        game_state->national_event_rounds = 2;

        printf("Construction suspended for 2 rounds.\n");
        break;

    case NE_INSURANCE_DISCOUNT:
        ne_mult->insurance_premium_mult = 80;

        printf("Premiums reduced by 20%%.\n");
        break;

    case NE_PROPERTY_REVALUATION:
        ne_mult->revalued_group = rand() % 8;
        ne_mult->revalued_group_mult = 115;

        printf("%s properties appreciate by 15%%.\n", group_name(ne_mult->revalued_group));
        break;

    case NE_CURRENCY_DEPRECIATION:
        ne_mult->construct_cost_mult = 110;

        printf("Construction costs increase by 10%%.\n");
        break;

    case NE_GOVERNMENT_GRANT:
        idx = rand() % 4;
        while (monopoly->players[idx].is_bankrupt) {
            idx = (idx + 1) % 4;
        }
        monopoly->players[idx].cash += 5000;

        printf("%s receives LKR 5000.\n", monopoly->players[idx].name);
        break;

    case NE_NATIONAL_DISASTER:
        target = random_owned_property(monopoly, 1);
        if (target == NULL) {
            printf("No developed property was damaged.\n");
            break;
        }
        damage_property(monopoly, target);

        printf("Affected Property :\n%s\n", target->name);
        break;

    default:
        break;
    }
    putchar('\n');
}


/*
draws the top regional development card every 15 rounds
*/
void draw_regional_card(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct RDMultipliers_S *rd_mult = &game_state->rd_mult;

    int card = deck_draw(&game_state->regional_deck);

    reset_rd_mult(monopoly);
    game_state->active_regional_card = card;
    game_state->regional_card_rounds = 15;

    printf("Regional Development\n");
    printf("%s\n", regional_card_name(card));

    switch (card) {
    case RD_SOUTH_TOUR_BOOM:
        rd_mult->south_rent_mult = 140;

        printf("Galle Fort, Unawatuna and Hikkaduwa rental income +40%%.\n");
        break;

    case RD_PORT_CITY_EXPANSION:
        rd_mult->colombo_val_mult = 125;

        printf("Pettah, Maradana and Colombo Fort Station values +25%%.\n");
        break;

    case RD_IT_INDUSTRY_GROWTH:
        rd_mult->it_zone_val_mult = 120;

        printf("Maharagama, Nugegoda and Kottawa values +20%%.\n");
        break;

    case RD_NORTH_DEV_PROGRAMME:
        rd_mult->north_val_mult = 130;

        printf("Jaffna Town, Nallur and Trincomalee values +30%%.\n");
        break;

    case RD_TEA_EXPORT_BOOM:
        rd_mult->hill_val_mult = 135;

        printf("Nuwara Eliya value +35%%.\n");
        break;

    case RD_AIRPORT_EXPANSION:
        rd_mult->airport_rent_mult = 130;

        printf("Negombo, Katunayake and Ja-Ela rents +30%%.\n");
        break;

    case RD_UNI_CITY_GROWTH:
        rd_mult->kandy_val_mult = 120;

        printf("Peradeniya and Kandy City values +20%%.\n");
        break;

    case RD_BEACH_POLLUTION:
        rd_mult->south_rent_mult = 70;

        printf("Southern coastal rents -30%%.\n");
        break;

    case RD_FLOOD_DAMAGE:
        rd_mult->coastal_val_mult = 80;

        printf("Low lying coastal properties lose 20%% value.\n");
        break;

    case RD_TRANSPORT_STRKE:
        rd_mult->rail_rent_mult = 60;

        printf("Railway revenue reduced by 40%%.\n");
        break;

    case RD_ELEC_TARRIF_INCREASE:
        rd_mult->util_rent_mult = 125;

        printf("Utility rent +25%%.\n");
        break;

    case RD_WATER_SHORTAGE:
        rd_mult->util_rent_mult = 120;
        rd_mult->water_area_val_mult = 90;

        printf("Water utility revenue +20%%.\n");
        printf("Surrounding properties -10%%.\n");
        break;

    default:
        break;
    }
    putchar('\n');
}


/*
picks a random government regulation every 20 rounds
*/
void draw_gov_regulation(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    int reg = rand() % 8;

    game_state->active_gov_reg = reg;
    game_state->gov_reg_rounds = 20;
    game_state->income_tax_rate = 15;

    printf("Government Regulation\n");
    printf("%s Introduced.\n", gov_regulation_name(reg));

    switch (reg) {
    case GR_INC_PROPERTY_TAX:
        game_state->income_tax_rate = 15 + 15 * 50 / 100;

        printf("Income Tax increases by 50%%.\n");
        break;

    case GR_REDUCE_LOAN_INTEREST:
        printf("Interest decreases by 2%%.\n");
        break;

    case GR_HOUSING_SUBSIDY:
        printf("Construction costs reduced by 30%%.\n");
        break;

    case GR_LUXURY_PROPERTY_TAX:
        printf("Hotels incur an annual maintenance tax of 25%%.\n");
        break;

    case GR_RAILWAY_MODERNIZATION:
        printf("Railway rents increase 25%%.\n");
        break;

    case GR_ELEC_TARRIF_REVISION:
        printf("Utility rents increase 20%%.\n");
        break;

    case GR_INSURANCE_REGULATION:
        printf("Insurance premiums decrease 15%%.\n");
        break;

    case GR_ANTI_SPECULATION_ACT:
        printf("Players may own at most three undeveloped properties.\n");
        break;

    default:
        break;
    }
    putchar('\n');
}


/*
picks a boom group and a decline group, both on a 30 round cooldown
*/
void market_review(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    for (int i = 0; i < 8; i++) {
        if (game_state->group_cooldown[i] > 0) game_state->group_cooldown[i]--;
    }

    int avail[8];
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (game_state->group_cooldown[i] == 0) avail[count++] = i;
    }
    if (count < 2) return;

    int boom = avail[rand() % count];
    int decline = avail[rand() % count];
    while (decline == boom) {
        decline = avail[rand() % count];
    }

    game_state->market_boom_group = boom;
    game_state->marker_boom_turns = 10;
    game_state->market_decline_group = decline;
    game_state->market_decline_turns = 10;

    game_state->group_cooldown[boom] = 30;
    game_state->group_cooldown[decline] = 30;

    printf("Market Boom\n");
    printf("%s (+20%%)\n\n", group_name(boom));
    printf("Market Decline\n");
    printf("%s (-15%%)\n\n", group_name(decline));
}


/*
counts down every active effect and clears the expired ones
*/
void expire_events(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct Square_S *board = monopoly->board;

    if (game_state->active_economic_event != EE_NONE) {
        game_state->economic_event_rounds--;
        if (game_state->economic_event_rounds <= 0) {
            printf("%s has ended.\n\n", economic_event_name(game_state->active_economic_event));
            reset_ee_mult(monopoly);
            game_state->active_economic_event = EE_NONE;
        }
    }

    if (game_state->active_national_event != NE_NONE) {
        game_state->national_event_rounds--;
        if (game_state->national_event_rounds <= 0) {
            printf("%s has expired.\n\n", national_event_name(game_state->active_national_event));
            reset_ne_mult(monopoly);
            game_state->active_national_event = NE_NONE;
            game_state->national_event_player = PL_NONE;
        }
    }

    if (game_state->active_regional_card != RD_NONE) {
        game_state->regional_card_rounds--;
        if (game_state->regional_card_rounds <= 0) {
            printf("%s has expired.\n\n", regional_card_name(game_state->active_regional_card));
            reset_rd_mult(monopoly);
            game_state->active_regional_card = RD_NONE;
        }
    }

    if (game_state->active_gov_reg != GR_NONE) {
        game_state->gov_reg_rounds--;
        if (game_state->gov_reg_rounds <= 0) {
            printf("%s has expired.\n\n", gov_regulation_name(game_state->active_gov_reg));
            game_state->active_gov_reg = GR_NONE;
            game_state->income_tax_rate = 15;
        }
    }

    if (game_state->marker_boom_turns > 0) {
        game_state->marker_boom_turns--;
        if (game_state->marker_boom_turns == 0) game_state->market_boom_group = PT_NONE;
    }

    if (game_state->market_decline_turns > 0) {
        game_state->market_decline_turns--;
        if (game_state->market_decline_turns == 0) game_state->market_decline_group = PT_NONE;
    }

    for (int i = 0; i < 40; i++) {
        if (board[i].type != ST_PROPERTY) continue;
        if (board[i].properties.closed_rounds > 0) board[i].properties.closed_rounds--;
    }
}


/*
prints the active market conditions at the end of a round
*/
void market_conditions(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    printf("=========================================\n");
    printf("Current Market Conditions\n");
    printf("=========================================\n\n");

    printf("Market Boom\n");
    printf("-------------\n");
    if (game_state->market_boom_group == PT_NONE) printf("None\n\n");
    else {
        printf("%s (+20%%)\n", group_name(game_state->market_boom_group));
        printf("Rounds Remaining : %d\n\n", game_state->marker_boom_turns);
    }

    printf("Market Decline\n");
    printf("----------------\n");
    if (game_state->market_decline_group == PT_NONE) printf("None\n\n");
    else {
        printf("%s (-15%%)\n", group_name(game_state->market_decline_group));
        printf("Rounds Remaining : %d\n\n", game_state->market_decline_turns);
    }

    printf("Regional Development\n");
    printf("-----------------------\n");
    if (game_state->active_regional_card == RD_NONE) printf("None\n\n");
    else {
        printf("%s\n", regional_card_name(game_state->active_regional_card));
        printf("Rounds Remaining : %d\n\n", game_state->regional_card_rounds);
    }

    printf("National Event\n");
    printf("----------------\n");
    if (game_state->active_national_event == NE_NONE) printf("None\n\n");
    else {
        printf("%s\n", national_event_name(game_state->active_national_event));
        printf("Rounds Remaining : %d\n\n", game_state->national_event_rounds);
    }

    printf("Government Regulation\n");
    printf("-----------------------\n");
    if (game_state->active_gov_reg == GR_NONE) printf("None\n\n");
    else {
        printf("%s\n", gov_regulation_name(game_state->active_gov_reg));
        printf("Rounds Remaining : %d\n\n", game_state->gov_reg_rounds);
    }

    printf("Inflation\n");
    printf("------------\n");
    printf("%d%%\n\n", game_state->current_inflation_rate);

    printf("Current Loan Interest\n");
    printf("-----------------------\n");
    printf("%d%%\n", game_state->current_interest_rate);
    printf("=========================================\n\n");
}


/*
market price of a square after every active event multiplier
*/
int property_value(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct GameState_S *game_state = &monopoly->game_state;
    struct EEMultipliers_S *ee_mult = &game_state->ee_mult;
    struct NEMultipliers_S *ne_mult = &game_state->ne_mult;
    struct RDMultipliers_S *rd_mult = &game_state->rd_mult;

    int value = sq->properties.market_price;

    if (sq->type == ST_RAILWAY) {
        value = value * ne_mult->rail_val_mult / 100;
        if (is_colombo_zone(sq)) value = value * rd_mult->colombo_val_mult / 100;
        return value;
    }

    if (sq->type != ST_PROPERTY) return value;

    value = value * ee_mult->prop_val_mult / 100;
    value = value * ne_mult->prop_val_mult / 100;

    if (is_south_coastal(sq)) value = value * ee_mult->south_prop_val_mult / 100;
    if (is_coastal(sq)) value = value * ee_mult->coastal_prop_val_mult / 100;
    if (is_commercial(sq)) value = value * ee_mult->commerce_prop_val / 100;
    if (is_commercial(sq)) value = value * ne_mult->commerce_prop_val_mult / 100;
    if (sq->properties.type == ne_mult->revalued_group) value = value * ne_mult->revalued_group_mult / 100;

    if (is_colombo_zone(sq)) value = value * rd_mult->colombo_val_mult / 100;
    if (is_it_zone(sq)) value = value * rd_mult->it_zone_val_mult / 100;
    if (is_north(sq)) value = value * rd_mult->north_val_mult / 100;
    if (is_kandy_zone(sq)) value = value * rd_mult->kandy_val_mult / 100;
    if (sq->id == BOARD_NUWARA_ELIYA) value = value * rd_mult->hill_val_mult / 100;
    if (is_coastal(sq)) value = value * rd_mult->coastal_val_mult / 100;
    if (game_state->active_regional_card == RD_WATER_SHORTAGE) value = value * rd_mult->water_area_val_mult / 100;

    if (sq->properties.type == game_state->market_boom_group) value = value * 120 / 100;
    if (sq->properties.type == game_state->market_decline_group) value = value * 85 / 100;

    return value;
}


/*
percentage applied to residential rent on a square
*/
int event_rent_mult(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.rent_mult;

    if (is_south_coastal(sq)) mult = mult * game_state->rd_mult.south_rent_mult / 100;
    if (is_airport_zone(sq)) mult = mult * game_state->rd_mult.airport_rent_mult / 100;

    if (sq->properties.type == game_state->market_boom_group) mult = mult * 125 / 100;
    if (sq->properties.type == game_state->market_decline_group) mult = mult * 80 / 100;

    return mult;
}


/*
percentage applied to hotel rent for an owner
*/
int event_hotel_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.hotel_rent_mult;
    if (owner->id == game_state->national_event_player) {
        mult = mult * game_state->ne_mult.hotel_rent_mult / 100;
    }
    return mult;
}


/*
percentage applied to railway rent for an owner
*/
int event_rail_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.rail_rent_mult;
    mult = mult * game_state->rd_mult.rail_rent_mult / 100;
    if (game_state->active_gov_reg == GR_RAILWAY_MODERNIZATION) mult = mult * 125 / 100;
    if (owner->id == game_state->national_event_player) {
        mult = mult * game_state->ne_mult.rail_rent_mult / 100;
    }
    return mult;
}


/*
percentage applied to utility rent for an owner
*/
int event_util_rent_mult(struct Monopoly_S *monopoly, struct Player_S *owner) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->rd_mult.util_rent_mult;
    if (game_state->active_gov_reg == GR_ELEC_TARRIF_REVISION) mult = mult * 120 / 100;
    if (owner->id == game_state->national_event_player) {
        mult = mult * game_state->ne_mult.util_income_mult / 100;
    }
    return mult;
}


/*
percentage applied to house construction cost
*/
int event_house_cost_mult(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.prop_dev_cost_mult;
    mult = mult * game_state->ee_mult.house_contruct_cost_mult / 100;
    if (game_state->active_gov_reg == GR_HOUSING_SUBSIDY) mult = mult * 70 / 100;
    if (player->id == game_state->national_event_player) {
        mult = mult * game_state->ne_mult.house_construct_cost_mult / 100;
        mult = mult * game_state->ne_mult.construct_cost_mult / 100;
    }
    return mult;
}


/*
percentage applied to hotel construction cost
*/
int event_hotel_cost_mult(struct Monopoly_S *monopoly, struct Player_S *player) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.prop_dev_cost_mult;
    if (game_state->active_gov_reg == GR_HOUSING_SUBSIDY) mult = mult * 70 / 100;
    if (player->id == game_state->national_event_player) {
        mult = mult * game_state->ne_mult.construct_cost_mult / 100;
    }
    return mult;
}


/*
percentage applied to an insurance premium
*/
int event_insurance_mult(struct Monopoly_S *monopoly) {
    struct GameState_S *game_state = &monopoly->game_state;

    int mult = game_state->ee_mult.insurance_premium_mult;
    mult = mult * game_state->ne_mult.insurance_premium_mult / 100;
    if (game_state->active_gov_reg == GR_INSURANCE_REGULATION) mult = mult * 85 / 100;
    return mult;
}


/*
1 if the square sits on the coast
*/
int is_coastal(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_BAMBALAPITIYA:
        case BOARD_WELLAWATTE:
        case BOARD_MOUNT_LAVINIA:
        case BOARD_NEGOMBO:
        case BOARD_KATUNAYAKE:
        case BOARD_GALLE_FORT:
        case BOARD_UNAWATUNA:
        case BOARD_HIKKADUWA:
        case BOARD_TRINCOMALEE:
        case BOARD_GALLE_FACE:
            return 1;
    }
    return 0;
}


/*
1 if the square is a southern coastal property
*/
int is_south_coastal(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_GALLE_FORT:
        case BOARD_UNAWATUNA:
        case BOARD_HIKKADUWA:
            return 1;
    }
    return 0;
}


/*
1 if the square is a commercial property
*/
int is_commercial(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_PETTAH:
        case BOARD_MARADANA:
        case BOARD_BAMBALAPITIYA:
        case BOARD_NUGEGODA:
        case BOARD_MAHARAGAMA:
        case BOARD_KOTTAWA:
        case BOARD_KANDY_CITY:
        case BOARD_JAFFNA_TOWN:
            return 1;
    }
    return 0;
}


/*
1 if the square is in the IT zone
*/
int is_it_zone(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_NUGEGODA:
        case BOARD_MAHARAGAMA:
        case BOARD_KOTTAWA:
            return 1;
    }
    return 0;
}


/*
1 if the square is a northern property
*/
int is_north(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_JAFFNA_TOWN:
        case BOARD_NALLUR:
        case BOARD_TRINCOMALEE:
            return 1;
    }
    return 0;
}


/*
1 if the square is in the airport zone
*/
int is_airport_zone(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_NEGOMBO:
        case BOARD_KATUNAYAKE:
        case BOARD_JA_ELA:
            return 1;
    }
    return 0;
}


/*
1 if the square is in the Kandy university zone
*/
int is_kandy_zone(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_KANDY_CITY:
        case BOARD_PERADENIYA:
            return 1;
    }
    return 0;
}


/*
1 if the square is in the Colombo port zone
*/
int is_colombo_zone(struct Square_S *sq) {
    switch (sq->id) {
        case BOARD_PETTAH:
        case BOARD_MARADANA:
        case BOARD_COLOMBO_FORT_STATION:
            return 1;
    }
    return 0;
}


/*
NULL if none, otherwise a random owned property
*/
struct Square_S *random_owned_property(struct Monopoly_S *monopoly, int developed_only) {
    struct Square_S *board = monopoly->board;

    struct Square_S *list[40];
    int count = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        struct Property_S *property = &sq->properties;
        if (property->current_owner == PL_NONE) continue;
        if (developed_only && property->house_count == 0 && property->hotel_count == 0) continue;
        list[count++] = sq;
    }

    if (count == 0) return NULL;
    return list[rand() % count];
}


/*
NULL if none, otherwise a random owned coastal property
*/
struct Square_S *random_coastal_property(struct Monopoly_S *monopoly) {
    struct Square_S *board = monopoly->board;

    struct Square_S *list[40];
    int count = 0;

    for (int i = 0; i < 40; i++) {
        struct Square_S *sq = &board[i];
        if (sq->type != ST_PROPERTY) continue;
        if (is_coastal(sq) == 0) continue;
        if (sq->properties.current_owner == PL_NONE) continue;
        list[count++] = sq;
    }

    if (count == 0) return NULL;
    return list[rand() % count];
}


/*
wrecks the buildings on a square and drops its market price
*/
void damage_property(struct Monopoly_S *monopoly, struct Square_S *sq) {
    struct Property_S *property = &sq->properties;

    for (int i = 0; i < property->house_count; i++) property->house_condition[i] = 0;
    if (property->hotel_count) property->hotel_condition = 0;

    property->market_price -= property->market_price * 15 / 100;
    property->depreciation_precent += 15;
    if (property->depreciation_precent > 30) property->depreciation_precent = 30;

    if (property->current_owner != PL_NONE) {
        property->is_damaged = 1;
        property->repair_cost = property->house_count * property->house_construction_cost / 2;
        property->repair_cost += property->hotel_count * property->hotel_construction_cost / 2;
        monopoly->players[property->current_owner].suffered_loss = 1;
    }
}


const char *economic_event_name(int event) {
    switch (event) {
        case EE_TOURISM_BOOM:                 return "Tourism Boom";
        case EE_FUEL_CRISIS:                  return "Fuel Crisis";
        case EE_HEAVY_MONSOON:                return "Heavy Monsoon";
        case EE_ECONOMIC_RECESSION:           return "Economic Recession";
        case EE_STOCK_MARKET_BOOM:            return "Stock Market Boom";
        case EE_GOVERNMENT_HOUSING_PROGRAMME: return "Government Housing Programme";
        case EE_FOREIGN_INVESTMENT:           return "Foreign Investment";
        case EE_POLITICAL_UNREST:             return "Political Unrest";
    }
    return "None";
}


const char *national_event_name(int card) {
    switch (card) {
        case NE_TOURISM_HYPE:           return "Tourism Hype";
        case NE_FUEL_SHORTAGE:          return "Fuel Shortage";
        case NE_HEAVY_FLOODS:           return "Heavy Floods";
        case NE_POLITICAL_RALLY:        return "Political Rally";
        case NE_STOCK_MARKET_RISE:      return "Stock Market Rise";
        case NE_ECONOMIC_DOWNTURN:      return "Economic Downturn";
        case NE_HOUSING_SUBSIDY:        return "Housing Subsidy";
        case NE_INTEREST_RATE_CUT:      return "Interest Rate Cut";
        case NE_INTEREST_RATE_INCREASE: return "Interest Rate Increase";
        case NE_TAX_AMNESTY:            return "Tax Amnesty";
        case NE_POWER_FAILURE:          return "Power Failure";
        case NE_FOREIGN_FUNDING:        return "Foreign Funding";
        case NE_PORT_EXPANSION:         return "Port Expansion";
        case NE_FESTIVAL_SEASON:        return "Festival Season";
        case NE_LABOUR_STRIKE:          return "Labour Strike";
        case NE_INSURANCE_DISCOUNT:     return "Insurance Discount";
        case NE_PROPERTY_REVALUATION:   return "Property Revaluation";
        case NE_CURRENCY_DEPRECIATION:  return "Currency Depreciation";
        case NE_GOVERNMENT_GRANT:       return "Government Grant";
        case NE_NATIONAL_DISASTER:      return "National Disaster";
    }
    return "None";
}


const char *regional_card_name(int card) {
    switch (card) {
        case RD_SOUTH_TOUR_BOOM:      return "Southern Tourism Boom";
        case RD_PORT_CITY_EXPANSION:  return "Port City Expansion";
        case RD_IT_INDUSTRY_GROWTH:   return "IT Industry Growth";
        case RD_NORTH_DEV_PROGRAMME:  return "Northern Development Programme";
        case RD_TEA_EXPORT_BOOM:      return "Tea Export Boom";
        case RD_AIRPORT_EXPANSION:    return "Airport Expansion";
        case RD_UNI_CITY_GROWTH:      return "University City Growth";
        case RD_BEACH_POLLUTION:      return "Beach Pollution";
        case RD_FLOOD_DAMAGE:         return "Flood Damage";
        case RD_TRANSPORT_STRKE:      return "Transport Strike";
        case RD_ELEC_TARRIF_INCREASE: return "Electricity Tariff Increase";
        case RD_WATER_SHORTAGE:       return "Water Shortage";
    }
    return "None";
}


const char *gov_regulation_name(int reg) {
    switch (reg) {
        case GR_INC_PROPERTY_TAX:      return "Increase Property Tax";
        case GR_REDUCE_LOAN_INTEREST:  return "Reduce Loan Interest";
        case GR_HOUSING_SUBSIDY:       return "Housing Subsidy";
        case GR_LUXURY_PROPERTY_TAX:   return "Luxury Property Tax";
        case GR_RAILWAY_MODERNIZATION: return "Railway Modernization";
        case GR_ELEC_TARRIF_REVISION:  return "Electricity Tariff Revision";
        case GR_INSURANCE_REGULATION:  return "Insurance Regulation";
        case GR_ANTI_SPECULATION_ACT:  return "Anti Speculation Act";
    }
    return "None";
}


const char *group_name(int group) {
    switch (group) {
        case PT_BROWN:      return "Brown";
        case PT_LIGHT_BLUE: return "Light Blue";
        case PT_PINK:       return "Pink";
        case PT_ORANGE:     return "Orange";
        case PT_RED:        return "Red";
        case PT_YELLOW:     return "Yellow";
        case PT_GREEN:      return "Green";
        case PT_DARK_BLUE:  return "Dark Blue";
    }
    return "None";
}
