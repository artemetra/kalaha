#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "game.h"

const int PLAYER1_HOME = 13;
const int PLAYER2_HOME = 6;
const int BOARD_BYTESIZE = 14;
const char BALL_CHAR[] = "\u25CF";

uint8_t* get_hole(Board* board, uint32_t idx) {
    return ((uint8_t*)board + (idx % BOARD_BYTESIZE));
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

    char buf[400];  // should be enough
    int tot = 0;
    tot += sprintf(buf, "\n\n");
    tot += sprintf(buf + tot, "|1    |2    |3    |4    |5    |6    |\n");
    tot += sprintf(buf + tot, "|");
    for (uint32_t i = 0; i < 6; i++)
        tot += sprintf(buf + tot, " %3d%s|", *get_hole(board, i), BALL_CHAR);
    tot += sprintf(buf + tot, "\n");

    tot += sprintf(buf + tot, "   ↑              →              ↓   \n");
    tot += sprintf(buf + tot, "|HOME1|                       |HOME2|\n");
    tot += sprintf(buf + tot, "| %3d%s|                       | %3d%s|\n", board->p1_home,
                   BALL_CHAR, board->p2_home, BALL_CHAR);
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

TurnOutcome make_a_turn(Board* board, uint32_t idx, Player player_id) {
    uint8_t* chosen_hole = get_hole(board, idx);
    if (*chosen_hole == 0) {
        return INVALID;
    }
    uint8_t in_hand = *chosen_hole;
    *chosen_hole = 0;

    // defined outside of the loop to have access afterwards
    uint8_t* hole = NULL;

    while (in_hand) {
        idx++;
        hole = get_hole(board, idx);
        // if the current hole is not current player's home, skip it
        if ((hole == &(board->p1_home) && player_id == 1) ||
            (hole == &(board->p2_home) && player_id == 0)) {
            continue;
        }
        (*hole)++;
        in_hand--;
    }
    // if the last ball landed in the player's home, the player gets
    // to make another turn
    if ((hole == &(board->p1_home) && player_id == 0) ||
        (hole == &(board->p2_home) && player_id == 1)) {
        return REPEAT;
    }
    assert(hole);
    if (*hole == 1) {
        return COMPLETE;
    } else {  // *hole > 1
        return make_a_turn(board, idx /*%BOARD_BYTESIZE*/, player_id);
    }
}

uint8_t sum(uint8_t holes[6]) {
    uint8_t total = 0;
    for (size_t i = 0; i < 6; i++)
        total += holes[i];
    return total;
}

void outcome_to_str(TurnOutcome outcome, char* output, size_t size) {
    switch (outcome)
    {
    case COMPLETE:
        strlcpy(output, "COMPLETE", size);
        break;
    case REPEAT:
        strlcpy(output, "REPEAT", size);
        break;
    case INVALID:
        strlcpy(output, "INVALID", size);
        break;
    default:
        strlcpy(output, "wut?", size);
        break;
    }
}
