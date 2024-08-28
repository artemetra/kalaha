#ifndef KALAHA_GAME_H_
#define KALAHA_GAME_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
    Player id, either `P1` (false) or `P2` (true).
*/
typedef bool Player;

/* Player 1 */
#define P1 0
/* Player 2 */
#define P2 1
/* Game ends in a draw */
#define DRAW -1

extern const int PLAYER1_HOME, PLAYER2_HOME, BOARD_BYTESIZE;
extern const char BALL_CHAR[];

/*
    The playing board of Kalaha.
    There are two types of holes: regular and "homes".
    Regular homes are used by players to do their turns on.
    "Homes" determine the scores each player gets, and are not directly accessible to
    the players, instead, they are filled up as the game progresses.
    The holes for player 1 are stored in reverse and after player 2's. The reason for
    this is to closer resemble the clockwise nature of the game and to make board accesses
    faster.
    The default board is such that:
    - all p1_holes are p2_holes are 6
    - p1_home and p2_home are both 0.
*/
typedef struct Board {
    // stored in left to right order, e.g. hole [0] is the leftmost hole on a real board
    uint8_t p2_holes[6];
    uint8_t p2_home;
    // stored in right to left order, e.g. hole [0] is the rightmost hole on a real board
    uint8_t p1_holes[6];
    uint8_t p1_home;
} Board;

/*
    Turn outcomes that are returned from `make_a_turn`.
*/
typedef enum TurnOutcome { COMPLETE, REPEAT, INVALID } TurnOutcome;

/*
    Writes the turn outcome to a string.

    `outcome` -- turn outcome to write.
    `output` (OUT) -- string output.
    `size` -- size arg for strlcpy.
*/
void outcome_to_str(TurnOutcome outcome, char* output, size_t size);

/*
    Returns a pointer to the contents of the hole given by `idx`, which is got from
    `convert_index`. Returns `NULL` if `idx` is invalid.
    The indexing is defined as follows:
    0-5: player2's holes 1-6
    6: player2's home
    7-12: player1's holes 1-6 (right to left/clockwise)
    13: player1's home

    `board` (IN) -- the board to access.
    `idx` -- index from `convert_index`.
*/
uint8_t* get_hole(Board* board, uint8_t idx);

/*
    Prints the board to the screen (stdout).

    `board` (IN) -- the board to print

    TODO: write to a string instead?
*/
void display_board(Board* board);

/*
    Converts index entered from the user into an internal one, for use in `make_a_turn`.
    `idx` -- index of the hole played (1-6)
    `player_id` -- id of the player that is playing.
*/
uint8_t convert_index(uint8_t idx, Player player_id);

/*
    Helper function to find the sum of balls in player's holes.
*/
uint8_t sum(uint8_t holes[6]);

/*
    Makes a turn on the board.

    `board` (IN/OUT) -- the board on which the turn is performed. `board` is modified.
    `idx` -- the index of the hole to pick.
    `player_id` -- who is doing the turn.
*/
TurnOutcome make_a_turn(Board* board, uint8_t idx, Player player_id);

#endif  // KALAHA_GAME_H_
