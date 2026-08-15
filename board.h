#ifndef BOARD_H
#define BOARD_H

#include "types.h"

void board_init(struct Square_S *square, int id, const char *name, enum SquareType_E type);

#endif