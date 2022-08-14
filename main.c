#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "colors.h"

#define _KALAHA_DEBUG
#define noop
#ifdef _KALAHA_DEBUG
    #define DEBUG printf
#else
    #define DEBUG noop
#endif

/* Player id, 0 or 1 */
typedef bool Player;

const int PLAYER1_HOME = 13;
const int PLAYER2_HOME = 6;
const int BOARD_BYTESIZE = 14;
const int MAX_INDEX = 14;
const char BALL_CHAR[] = "\u25CF";

typedef struct Board {
    uint8_t p2_holes[6];
    uint8_t p2_home;
    uint8_t p1_holes[6];  // stored in right to left order
    uint8_t p1_home;
} Board;

/*
    Retrieves a pointer to a hole by index. The indexing is defined as follows:
    0-5: player2's holes 1-6
    6: player2's home
    7-12: player1's holes 1-6 (right to left/clockwise)
    13: player1's home
*/
uint8_t* get_hole(Board* board, int idx) {
    return ((uint8_t*)board + (idx%BOARD_BYTESIZE));
}

static uint32_t board_sum(Board* board) {
    uint32_t total = 0;
    for (size_t i = 0; i < 14; i++)
        total += (uint32_t)(*get_hole(board, i));
    return total;
}

// TODO: refactor whatever this is
void display_board(Board* board) {
    char buf[400]; // should be enough 
    int tot = 0;
    tot += sprintf(buf, "\n\n|");
    for (int i = 0; i < 6; i++)
        tot += sprintf(buf + tot, "%d    |", i + 1);
    tot += sprintf(buf + tot, "\n|");
    for (int i = 0; i < 6; i++)
        tot += sprintf(buf + tot, " %3d%s|", *get_hole(board, i), BALL_CHAR);
    tot += sprintf(buf + tot, "\n");
    tot += sprintf(buf + tot, "   ↑              →              ↓   \n");
    tot += sprintf(buf + tot, "|HOME1|                       |HOME2|");
    tot += sprintf(buf + tot, "\n");
    tot += sprintf(buf + tot, "| %3d%s|                       | %3d%s|\n",
                   board->p1_home, BALL_CHAR, board->p2_home, BALL_CHAR);
    tot += sprintf(buf + tot, "   ↑              ←              ↓   \n");
    tot += sprintf(buf + tot, "|");
    for (int i = 6; i > 0; i--)
        tot += sprintf(buf + tot, "%d    |", i);
    tot += sprintf(buf + tot, "\n|");
    for (int i = PLAYER1_HOME-1; i > PLAYER2_HOME; i--)
        tot += sprintf(buf + tot, " %3d%s|", *get_hole(board, i), BALL_CHAR);

    printf("%s\n", buf);
}

int convert_index(int idx, Player player_id) {
    if (player_id) 
        return idx - 1;
    else
        return idx + 6;
}

#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2
// shamelessly stolen from
// https://stackoverflow.com/questions/4023895/how-do-i-read-a-string-entered-by-the-user-in-c/4023921#4023921
static int get_line(char* prmpt, char* buff, size_t sz) {
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf("%s", prmpt);
        fflush(stdout);
    }
    if (fgets(buff, sz, stdin) == NULL)
        return NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff) - 1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = '\0';
    return OK;
}

/*
    Prompts a player to choose a hole.
*/
int prompt_user(Board* board, Player player_id) {
    char buffer[3];  // one digit, \n and \0
    char query[39];
    sprintf(query, "Player %d, choose hole to play (1-6): ", player_id+1);
    for (;;) {
        if (!get_line(query, buffer, 4)) {
            int user_input;
            // includes EOF and failure to match (0)
            if (sscanf(buffer, "%d", &user_input) > 0) {
                if (user_input >= 1 && user_input <= 6) {
                    int converted = convert_index(user_input, player_id);
                    if (*get_hole(board, converted) > 0)
                        return converted;
                    goto empty;
                }
            }
            goto invalid;
        }
    invalid:
        printf("Invalid input, try again\n");
        continue;
    empty:
        printf("The chosen hole is empty\n");
    }
}

#define COMPLETE 0
#define REPEAT 1
int make_a_turn(Board* board, uint32_t idx, Player player_id) {
    uint8_t* chosen_hole = get_hole(board, idx);
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
            DEBUG("         skipping player's home, hole==%d, &(board->p1_home)==%d; &(board->p2_home)==%d\n", (uintptr_t)hole, (uintptr_t)&(board->p1_home), (uintptr_t)&(board->p2_home));
            continue;
        }
        (*hole)++;
        in_hand--;
        //DEBUG("in_hand: %d, idx: %d, get_hole: %d;\n", in_hand, idx, *hole);
    }
    DEBUG("p1_home=%d; p2_home=%d; board_sum=%d; idx=%d;\n", board->p1_home, board->p2_home, board_sum(board), idx);
    
    // if the last ball landed in the player's home, the player gets
    // to make another turn
    if ((hole == &(board->p1_home) && player_id == 0) ||
        (hole == &(board->p2_home) && player_id == 1)) 
    {
        return REPEAT;
    }
    if (*hole==1) {
        return COMPLETE; 
    } else if ((*hole)>1) {
        return make_a_turn(board, idx/*%BOARD_BYTESIZE*/, player_id);
    }

}

void display_current_player(Player player_id) {
    if (player_id == 0)
    {
        printf("Current: Player 1\n");
    } else {
        printf("Current: Player 2\n");
    }
}

/*
    Helper function to find the sum of balls in holes.
*/
static uint32_t sum(uint8_t holes[6]) {
    uint8_t total = 0;
    for (size_t i = 0; i < 6; i++)
        total += holes[i];
    return total;
}



/*
    Main game loop. Returns the id of the winner 0 or 1
    -1 is the draw outcome
*/
int game_loop(Board* board) {
    Player player1 = 0;
    Player player2 = 1;
    Player current_player = player1;
    display_board(board);
    for (;;)
    {
        //display_current_player(current_player);
        int turn_outcome = make_a_turn(board, prompt_user(board, current_player), current_player);
        display_board(board);
        if (turn_outcome == COMPLETE) {
            printf("Player %d has finished their turn!\n", current_player+1);
            if ((current_player == 0 && sum(board->p1_holes) == 0) ||
                (current_player == 1 && sum(board->p2_holes) == 0))
            {
                printf("Player %d doesn't have any balls left in their holes, game over\n", current_player+1);
                break;
            }
            current_player = !current_player;
        } 
        else if (turn_outcome == REPEAT)
        {
            printf("Player %d landed their last ball into their home", current_player+1);
            if ((current_player == 0 && sum(board->p1_holes) == 0) ||
                (current_player == 1 && sum(board->p2_holes) == 0))
            {
                printf(", but they don't have any balls left in their holes, the game is over\n");
                break;
            } else {
                printf("! They get an extra turn.\n");

            }
        }
    }
    
    uint8_t p1_home = board->p1_home;
    uint8_t p2_home = board->p2_home;
    printf("---RESULTS---");
    printf("Player 1 has scored %d balls in their home\n", p1_home);
    printf("Player 2 has scored %d balls in their home\n", p2_home);
    if (p1_home > p2_home) {
        printf("Player 1 wins!\n");
        return player1;
    }
    else if (p2_home > p1_home) {
        printf("Player 2 wins!\n");
        return player2;
    }
    else {
        printf("Draw!");
        return -1;
    }
}


int main() {
    Board _board = {
        6, 6, 6, 6, 6, 6, // player2's holes
        0,                // player2's home
        6, 6, 6, 6, 6, 6, // player1's holes
        0                 // player1's home
    };
    // Board _board = {
    //     3, 3, 1, 2, 1, 4,
    //     12,
    //     2, 0, 5, 16, 4, 1,
    //     18
    // };

    Board* board = &_board;
    Player winner = game_loop(board);
    return 0;
}

