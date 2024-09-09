#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

const int PLAYER1_HOME = 13;
const int PLAYER2_HOME = 6;
const int BOARD_BYTESIZE = 14;
const char BEAD_CHAR[] = "\u25CF";

uint8_t* get_pit(Board* board, uint8_t idx) {
    if (idx < 0 || (idx % BOARD_BYTESIZE) > 13) {
        return NULL;
    }
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
        tot += sprintf(buf + tot, " %3d%s|", *get_pit(board, i), BEAD_CHAR);
    tot += sprintf(buf + tot, "\n");

    tot += sprintf(buf + tot, "   ↑              →              ↓   \n");
    tot += sprintf(buf + tot, "|HOME1|                       |HOME2|\n");
    tot += sprintf(buf + tot, "| %3d%s|                       | %3d%s|\n", board->p1_home,
                   BEAD_CHAR, board->p2_home, BEAD_CHAR);
    tot += sprintf(buf + tot, "   ↑              ←              ↓   \n");

    tot += sprintf(buf + tot, "|6    |5    |4    |3    |2    |1    |\n");
    tot += sprintf(buf + tot, "|");
    // print in reverse
    for (int i = 12; i > 6; i--)
        tot += sprintf(buf + tot, " %3d%s|", *get_pit(board, i), BEAD_CHAR);

    printf("%s\n", buf);
}

uint8_t convert_index(uint8_t idx, Player player_id) {
    if (player_id == P2)
        return idx - 1;
    else
        return idx + 6;
}

TurnOutcome make_a_turn(Board* board, uint8_t idx, Player player_id) {
    uint8_t* chosen_pit = get_pit(board, idx);
    // careful shortcircuiting
    if (chosen_pit == NULL || *chosen_pit == 0) {
        return INVALID;
    }
    assert(chosen_pit);
    uint8_t in_hand = *chosen_pit;
    *chosen_pit = 0;

    // defined outside of the loop to have access afterwards
    uint8_t* pit = NULL;

    while (in_hand) {
        idx++;
        idx %= BOARD_BYTESIZE;
        pit = get_pit(board, idx);
        // if the current pit is not current player's home, skip it
        if ((pit == &(board->p1_home) && player_id == 1) ||
            (pit == &(board->p2_home) && player_id == 0)) {
            continue;
        }
        (*pit)++;
        in_hand--;
    }
    // if the last bead landed in the player's home, the player gets
    // to make another turn
    if ((pit == &(board->p1_home) && player_id == 0) ||
        (pit == &(board->p2_home) && player_id == 1)) {
        return REPEAT;
    }
    assert(pit);
    if (*pit == 1) {
        return COMPLETE;
    } else {  // *pit > 1
        return make_a_turn(board, idx, player_id);
    }
}

uint8_t sum(uint8_t pits[6]) {
    uint8_t total = 0;
    for (size_t i = 0; i < 6; i++)
        total += pits[i];
    return total;
}

void outcome_to_str(TurnOutcome outcome, char* output, size_t size) {
    switch (outcome) {
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
