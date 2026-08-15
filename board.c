#include "board.h"
#include "types.h"
#include <string.h>

void board_init(struct Square_S *square, int id, const char *name, enum SquareType_E type) {
    square->id = id;
    strncpy(square->name, name, sizeof(square->name) - 1);
    square->type = type;
}