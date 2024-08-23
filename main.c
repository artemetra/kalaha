#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "colors.h"
#include "game.h"
#include "solver.h"

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

// TODO: make an option to leave (maybe)
/*
    Prompts a player to choose a hole.
*/
uint32_t prompt_user(Board* board, Player player_id) {
    char buffer[3];  // one digit, \n and \0
    char query[39];
    sprintf(query, "Player %d, choose hole to play (1-6): ", player_id + 1);
    for (;;) {
        if (!get_line(query, buffer, 4)) {
            int user_input;
            // includes EOF and failure to match (0)
            if (sscanf(buffer, "%d", &user_input) > 0) {
                if (user_input >= 1 && user_input <= 6) {
                    uint32_t converted = convert_index(user_input, player_id);
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

// TODO: refactor to make it solver friendly (make another function?)
/*
    Main game loop. Returns the id of the winner 0 or 1
    -1 is the draw outcome
*/
int game_loop(Board* board) {
    Player player1 = 0;
    Player player2 = 1;
    Player current_player = player1;
    display_board(board);
    uint8_t p1_holes_sum;
    uint8_t p2_holes_sum;
    for (;;) {
        TurnOutcome turn_outcome =
            make_a_turn(board, prompt_user(board, current_player), current_player);
        p1_holes_sum = sum(board->p1_holes);
        p2_holes_sum = sum(board->p2_holes);
        display_board(board);
        if (turn_outcome == COMPLETE) {
            printf("Player %d has finished their turn!\n", current_player + 1);
            if ((current_player == 0 && p1_holes_sum == 0) ||
                (current_player == 1 && p2_holes_sum == 0)) {
                printf("Player %d's holes are empty, game over\n", current_player + 1);
                break;
            }
            current_player = !current_player;
        } else if (turn_outcome == REPEAT) {
            printf("Player %d landed their last ball into their home",
                   current_player + 1);
            if ((current_player == 0 && p1_holes_sum == 0) ||
                (current_player == 1 && p2_holes_sum == 0)) {
                printf(", but their holes are empty, the game is over\n");
                break;
            } else {
                printf("! They get an extra turn.\n");
            }
        }
    }

    uint8_t p1_score = board->p1_home + p2_holes_sum;
    uint8_t p2_score = board->p2_home + p1_holes_sum;

    printf("\n---RESULTS---\n");
    printf(
        "Player 1 has scored %d points (%d in home + %2d of player 2's "
        "holes)\n",
        p1_score, board->p1_home, p2_holes_sum);
    printf(
        "Player 2 has scored %d points (%d in home + %2d of player 1's "
        "holes)\n",
        p2_score, board->p2_home, p1_holes_sum);

    if (p1_score > p2_score) {
        printf("Player 1 wins! Thanks for playing the game.\n");
        return player1;
    } else if (p2_score > p1_score) {
        printf("Player 2 wins! Thanks for playing the game.\n");
        return player2;
    } else {
        printf("Draw! Thanks for playing the game.\n");
        return -1;
    }
}

// for solver, unusued?
int alt_game_loop(Board* board) {
    StateNode* statenode = create_statenode(*board, 1);
    printf("statenode:\n");
    for (size_t i = 0; i < sizeof(statenode); i++) {
        printf("%02x ", ((uint8_t*)&statenode)[i]);
    }
    printf("\n");
    return 0;
}

int main() {
    Board _board = {
        {6, 6, 6, 6, 6, 6},  // player2's holes
        0,                   // player2's home
        {6, 6, 6, 6, 6, 6},  // player1's holes
        0                    // player1's home
    };
    /*
    |1    |2    |3    |4    |5    |6    |
    |   0●|   7●|   7●|   7●|   7●|   7●|
       ↑              →              ↓
    |HOME1|                       |HOME2|
    |   1●|                       |   0●|
       ↑              ←              ↓
    |6    |5    |4    |3    |2    |1    |
    |   7●|   7●|   7●|   7●|   1●|   7●|
    */
    Board _alt_board = {
        {0, 0, 7, 7, 0, 7},  // player2's holes
        0,                   // player2's home
        {7, 1, 7, 0, 0, 7},  // player1's holes
        1                    // player1's home
    };
    Board* board = &_alt_board;
    // game_loop(board);
    TurnOutcome* tos = malloc(6*sizeof(TurnOutcome));
    uint8_t holes[6] = {1,2,3,4,5,6};
    try_plays(board, PLAYER1, holes, 6, tos);
    for (int i=0; i<6; i++) {
        char buf[10];
        outcome_to_str(tos[i], buf, 10);
        printf("%s ", buf);
    }
    printf("\n");
    return 0;
}
