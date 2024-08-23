#ifndef KALAHA_GAME_H_
#define KALAHA_GAME_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Player id, 0 or 1 */
typedef bool Player;

#define P1 0
#define P2 1

extern const int PLAYER1_HOME, PLAYER2_HOME, BOARD_BYTESIZE;
extern const char BALL_CHAR[];

typedef struct Board {
    uint8_t p2_holes[6];
    uint8_t p2_home;
    uint8_t p1_holes[6];  // stored in right to left order
    uint8_t p1_home;
} Board;

typedef enum TurnOutcome { COMPLETE, REPEAT, INVALID } TurnOutcome;

void outcome_to_str(TurnOutcome outcome, char* output, size_t size);
/*
    Retrieves a pointer to a hole by index. The indexing is defined as follows:
    0-5: player2's holes 1-6
    6: player2's home
    7-12: player1's holes 1-6 (right to left/clockwise)
    13: player1's home
*/
uint8_t* get_hole(Board* board, uint32_t idx);

void display_board(Board* board);

/*
    Converts index entered from the user into an internal one
*/
int convert_index(int idx, Player player_id);

/*
    Helper function to find the sum of balls in player's holes.
*/
uint8_t sum(uint8_t holes[6]);

/*
    Makes a turn.
    Returns COMPLETE if the turn ended normally,
    Returns REPEAT if the player got an extra turn.
*/
TurnOutcome make_a_turn(Board* board, uint32_t idx, Player player_id);

#endif  // KALAHA_GAME_H_
