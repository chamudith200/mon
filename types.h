#ifndef TYPES_H
#define TYPES_H

#define DBG printf("cat\n");

enum BoardPosition {
    BOARD_GO = 0,
    BOARD_PETTAH,
    BOARD_COMMUNITY_FUND,
    BOARD_MARADANA,
    BOARD_INCOME_TAX,
    BOARD_COLOMBO_FORT_STATION,
    BOARD_BAMBALAPITIYA,
    BOARD_NATIONAL_EVENT_1,
    BOARD_WELLAWATTE,
    BOARD_MOUNT_LAVINIA,
    BOARD_JAIL_VISITING,
    BOARD_NUGEGODA,
    BOARD_CEB,
    BOARD_MAHARAGAMA,
    BOARD_KOTTAWA,
    BOARD_KANDY_STATION,
    BOARD_NEGOMBO,
    BOARD_SRI_LANKA_INSURANCE,
    BOARD_KATUNAYAKE,
    BOARD_JA_ELA,
    BOARD_FREE_PARKING,
    BOARD_KANDY_CITY,
    BOARD_NATIONAL_EVENT_2,
    BOARD_PERADENIYA,
    BOARD_KATUGASTOTA,
    BOARD_GALLE_STATION,
    BOARD_GALLE_FORT,
    BOARD_UNAWATUNA,
    BOARD_NWSDB,
    BOARD_HIKKADUWA,
    BOARD_GO_TO_JAIL,
    BOARD_JAFFNA_TOWN,
    BOARD_NALLUR,
    BOARD_CEYLINCO_INSURANCE,
    BOARD_TRINCOMALEE,
    BOARD_JAFFNA_STATION,
    BOARD_NATIONAL_EVENT_3,
    BOARD_NUWARA_ELIYA,
    BOARD_BANK_OF_CEYLON,
    BOARD_GALLE_FACE
};

enum SquareType_E {
    ST_START = 0,
    ST_PROPERTY,
    ST_EVENT,
    ST_TAX,
    ST_RAILWAY,
    ST_UTILITY,
    ST_INSURANCE,
    ST_BANK,
    ST_SPECIAL
};

enum PropertyType_E {
    PT_NONE = -1,
    PT_BROWN,
    PT_LIGHT_BLUE,
    PT_PINK,
    PT_ORANGE,
    PT_RED,
    PT_YELLOW,
    PT_GREEN,
    PT_DARK_BLUE
};

enum InsuranceType_E {
    INS_NONE = 0,
    INS_BASIC,
    INS_COMPREHENSIVE,
    INS_BUSINESS_INTERRUPTION
};

enum Disaster_E {
    DIS_NONE = -1,
    DIS_FIRE,
    DIS_FLOOD,
    DIS_RIOT,
    DIS_BUILDING_COLLAPSE,
    DIS_ELECTRICAL_FAILURE
};

struct Property_S {
    int base_price;
    int purchase_price;
    int market_price;

    int mortgage_val;
    int base_rental;
    int current_rent;

    int house_construction_cost;
    int hotel_construction_cost;
    
    enum PropertyType_E type;
    int current_owner;

    int is_mortaged;
    int is_loan_locked;

    int house_count;
    int hotel_count;
    int house_condition[4];
    int hotel_condition;
    int property_age;
    int depreciation_precent;

    int insurance_type;
    int insuarance_rounds;

    int closed_rounds;

    int is_damaged;
    int repair_cost;
    int missed_maintenance;
    int maintenance_cost_mult;
};

struct Square_S {
    int id;
    char name[64];

    enum SquareType_E type;
    struct Property_S properties;
};

enum PlayerType_E {
    PL_NONE = -1,
    PL_AGGRESIVE_INVESTOR,
    PL_CONSERVATIVE_BANKER,
    PL_RISK_TAKER,
    PL_OPPORTUNISTIC_TRADER
};

struct Player_S{
    int id;
    char name[32];
    enum PlayerType_E type;
    
    int current_pos;
    int is_bankrupt;
    int in_jail;
    int jail_turns;
    int passed_go;

    int cash;

    int has_active_loan;
    int loan_amount;
    int accrued_interest;
    int loan_interest_rate;
    int loan_round_remaining;

    int railway_owned;
    int util_owned;
    int property_owned;
    int hotels_owned;

    int property_val;
    int building_val;
    int railway_val;
    int utilitty_val;
    int insurance_claim_recivable;
    int taxes_due;

    int net_worth;
};

enum EconomicEvent_E {
    EE_NONE = -1,
    EE_TOURISM_BOOM,
    EE_FUEL_CRISIS,
    EE_HEAVY_MONSOON,
    EE_ECONOMIC_RECESSION,
    EE_STOCK_MARKET_BOOM,
    EE_GOVERNMENT_HOUSING_PROGRAMME,
    EE_FOREIGN_INVESTMENT,
    EE_POLITICAL_UNREST
};


struct EESnapshot_S {
    int property_price_change[40];
    int rent_change[40];
    int house_construction_cost[40];
    int hotel_construction_cost[40];

    int loan_interrest_rate;
    int flood_risk;
    int riot_probability;
    int business_interruption_claims;
};

struct EEMultipliers_S{
    int hotel_rent_mult;
    int south_prop_val_mult;

    int rail_rent_mult;
    int prop_dev_cost_mult;
    
    int insurance_premium_mult;
    int coastal_prop_val_mult;
    
    int prop_val_mult;  
    int rent_mult;
    int loan_interest_mult;

    int house_contruct_cost_mult;

    int commerce_prop_val;

    int riot_prob_mult;
    int business_interrupt_claim_mult;

    struct EESnapshot_S snapshot;
};

enum NationalEventCard_E {
    NE_NONE = -1,
    NE_TOURISM_HYPE,
    NE_FUEL_SHORTAGE,
    NE_HEAVY_FLOODS,
    NE_POLITICAL_RALLY,
    NE_STOCK_MARKET_RISE,
    NE_ECONOMIC_DOWNTURN,
    NE_HOUSING_SUBSIDY,
    NE_INTEREST_RATE_CUT,
    NE_INTEREST_RATE_INCREASE,
    NE_TAX_AMNESTY,
    NE_POWER_FAILURE,
    NE_FOREIGN_FUNDING,
    NE_PORT_EXPANSION,
    NE_FESTIVAL_SEASON,
    NE_LABOUR_STRIKE,
    NE_INSURANCE_DISCOUNT,
    NE_PROPERTY_REVALUATION,
    NE_CURRENCY_DEPRECIATION,
    NE_GOVERNMENT_GRANT,
    NE_NATIONAL_DISASTER
};

struct Deck_S {
    int cards[20];
    int count;
};

struct NEMultipliers_S {
    int hotel_rent_mult;
    int rail_rent_mult;
    int util_income_mult;

    int prop_val_mult;
    int commerce_prop_val_mult;
    int rail_val_mult;

    int house_construct_cost_mult;
    int construct_cost_mult;
    int insurance_premium_mult;

    int loan_interest_change;
    int construction_suspended;
};

enum RegionalDevCard_E {
    RD_NONE = -1,
    RD_SOUTH_TOUR_BOOM,
    RD_PORT_CITY_EXPANSION,
    RD_IT_INDUSTRY_GROWTH,
    RD_NORTH_DEV_PROGRAMME,
    RD_TEA_EXPORT_BOOM,
    RD_AIRPORT_EXPANSION,
    RD_UNI_CITY_GROWTH,
    RD_BEACH_POLLUTION,
    RD_FLOOD_DAMAGE,
    RD_TRANSPORT_STRKE,
    RD_ELEC_TARRIF_INCREASE,
    RD_WATER_SHORTAGE
};

struct RDMultipliers_S {
    int south_rent_mult;
    int colombo_val_mult;
    int it_zone_val_mult;
    int north_val_mult;
    int hill_val_mult;
    int airport_rent_mult;
    int kandy_val_mult;

    int coastal_val_mult;
    int rail_rent_mult;
    int util_rent_mult;
    int water_area_val_mult;
};

enum GovRegulation_E {
    GR_NONE = -1,
    GR_INC_PROPERTY_TAX,
    GR_REDUCE_LOAN_INTEREST,
    GR_HOUSING_SUBSIDY,
    GR_LUXURY_PROPERTY_TAX,
    GR_RAILWAY_MODERNIZATION,
    GR_ELEC_TARRIF_REVISION,
    GR_INSURANCE_REGULATION,
    GR_ANTI_SPECULATION_ACT
};

struct GameState_S {
    int current_turn;
    int current_round;
    int current_player;
    int current_inflation_rate;
    int current_interest_rate;
    int bankrupt_count;

    int income_tax_rate;
    int community_dev_fund_tax_rate;

    int active_national_event;
    int national_event_rounds;
    int national_event_player;

    int active_regional_card;
    int regional_card_rounds;

    int active_gov_reg;
    int gov_reg_rounds;

    int active_economic_event;
    int economic_event_rounds;

    int market_boom_group;
    int marker_boom_turns;

    int market_decline_group;
    int market_decline_turns;

    int group_cooldown[8];

    int turn_order[4];
    int current_turn_order;

    struct EEMultipliers_S ee_mult;
    struct NEMultipliers_S ne_mult;
    struct RDMultipliers_S rd_mult;

    struct Deck_S national_deck;
    struct Deck_S regional_deck;
};

struct Monopoly_S {
    struct Player_S players[4];
    struct Square_S board[40];
    struct GameState_S game_state;
};

#endif