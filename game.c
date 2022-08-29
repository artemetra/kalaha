#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

const int PLAYER1_HOME = 13;
const int PLAYER2_HOME = 6;
const int BOARD_BYTESIZE = 14;
const int MAX_INDEX = 14;
const char BALL_CHAR[] = "\u25CF";

uint8_t* get_hole(Board* board, uint32_t idx) {
    return ((uint8_t*)board + (idx%BOARD_BYTESIZE));
}
 
void display_board(Board* board) {
    /*
        |1    |2    |3    |4    |5    |6    |
        |   6●|   6●|   6●|   6●|   6●|   6●|
           ↑              →              ↓
        |HOME1|                       |HOME2|
        |   0●|                       |   0●|
           ↑              ←              ↓
        |6    |5    |4    |3    |2    |1    |
        |   6●|   6●|   6●|   6●|   6●|   6●|
    */

    char buf[400]; // should be enough 
    int tot = 0;
    tot += sprintf(buf, "\n\n");
    tot += sprintf(buf + tot, "|1    |2    |3    |4    |5    |6    |\n");
    tot += sprintf(buf + tot, "|");
    for (uint32_t i = 0; i < 6; i++)
        tot += sprintf(buf + tot, " %3d%s|", *get_hole(board, i), BALL_CHAR);
    tot += sprintf(buf + tot, "\n");

    tot += sprintf(buf + tot, "   ↑              →              ↓   \n");
    tot += sprintf(buf + tot, "|HOME1|                       |HOME2|\n");
    tot += sprintf(buf + tot, "| %3d%s|                       | %3d%s|\n",
                   board->p1_home, BALL_CHAR, board->p2_home, BALL_CHAR);
    tot += sprintf(buf + tot, "   ↑              ←              ↓   \n");

    tot += sprintf(buf + tot, "|6    |5    |4    |3    |2    |1    |\n");
    tot += sprintf(buf + tot, "|");
    for (int i = 12; i > 6; i--)
        tot += sprintf(buf + tot, " %3d%s|", *get_hole(board, i), BALL_CHAR);

    printf("%s\n", buf);
}

int convert_index(int idx, Player player_id) {
    if (player_id) 
        return idx - 1;
    else
        return idx + 6;
}


