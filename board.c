#include "board.h"
#include "types.h"
#include "game.h"

#include <stdio.h>
#include <string.h>


/*
sets the id, name and type of a single square
*/
void board_init(struct Square_S *square, int id, const char *name, enum SquareType_E type) {
    square->id = id;
    strncpy(square->name, name, sizeof(square->name) - 1);
    square->name[sizeof(square->name) - 1] = '\0';
    square->type = type;

    memset(&square->properties, 0, sizeof(square->properties));
    square->properties.type = PT_NONE;
    square->properties.current_owner = PL_NONE;
    square->properties.maintenance_cost_mult = 100;
}


/*
names and types every square on the board
*/
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


/*
sets prices, rents and build costs for every property group
*/
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


/*
sets ownership and price for railways and utilities
*/
void init_railways_utils(struct Square_S board[40]) {
    for (int i = 0; i < 40; i++) {
        if ((board[i].type != ST_RAILWAY) && (board[i].type != ST_UTILITY)) {
            continue;
        }

        struct Property_S *property = &board[i].properties;

        property->type = PT_NONE;
        property->current_owner = PL_NONE;

        property->base_price = 2000;
        property->purchase_price = 2000;
        property->market_price = 2000;
        property->mortgage_val = 1000;

        property->base_rental = 0;
        property->current_rent = 0;

        property->house_construction_cost = 0;
        property->hotel_construction_cost = 0;

        property->is_mortaged = 0;
        property->is_loan_locked = 0;

        property->house_count = 0;
        property->hotel_count = 0;
        for (int j = 0; j < 4; j++) property->house_condition[j] = 100;
        property->hotel_condition = 100;
        property->property_age = 0;
        property->depreciation_precent = 0;

        property->insurance_type = INS_NONE;
        property->insuarance_rounds = 0;

        property->closed_rounds = 0;
        property->is_damaged = 0;
        property->repair_cost = 0;
        property->missed_maintenance = 0;
        property->maintenance_cost_mult = 100;
    }
}


/*
rolls two dice and advances the current player, handles jail and GO
*/
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
