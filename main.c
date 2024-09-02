#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "colors.h"
#include "game.h"
#include "solver.h"

#define INPUT_OK 0
#define NO_INPUT 1
#define TOO_LONG 2

#define PVP 1
#define PVC 2

typedef bool GameOutcome;
#define GAME_OVER true
#define GAME_NEXT false

// TODO: rename
typedef struct TurnState {
    TurnOutcome turn_outcome;
    GameOutcome game_outcome;
} TurnState;

// Get a line from the user with a prompt.
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
        return (extra == 1) ? TOO_LONG : INPUT_OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff) - 1] = '\0';
    return INPUT_OK;
}

// TODO: make an option to leave (maybe)
/*
    Prompts a player to choose a hole to play.
*/
uint32_t prompt_user(Board* board, Player player_id) {
    char buffer[3];  // one digit, \n and \0
    char query[39];
    sprintf(query, "Player %d, choose hole to play (1-6): ", player_id + 1);
    for (;;) {
        int err = get_line(query, buffer, 4);

        // int rand_input = (arc4random() % 6) + 1;
        // printf("%d\n", rand_input);
        // sprintf(buffer, "%d\n", rand_input);
        // int err = 0;
        if (!err) {
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

bool is_win(Board* board) {
    uint32_t p1_holes_sum = sum(board->p1_holes);
    uint32_t p2_holes_sum = sum(board->p2_holes);
    return (p1_holes_sum == 0) || (p2_holes_sum == 0);
}

TurnState player_turn(Board* board,
                      Player current_player,
                      uint8_t* p1_holes_sum,
                      uint8_t* p2_holes_sum) {
    TurnOutcome turn_outcome =
        make_a_turn(board, prompt_user(board, current_player), current_player);

    *p1_holes_sum = sum(board->p1_holes);
    *p2_holes_sum = sum(board->p2_holes);

    TurnState turn_state;
    turn_state.turn_outcome = turn_outcome;
    turn_state.game_outcome = GAME_NEXT;  // until changed otherwise
    display_board(board);
    if (turn_outcome == COMPLETE) {
        printf("Player %d has finished their turn!\n", current_player + 1);
        if (is_win(board)) {
            printf("Player %d's holes are empty, game over\n", current_player + 1);
            turn_state.game_outcome = GAME_OVER;
        }
    } else if (turn_outcome == REPEAT) {
        printf("Player %d landed their last ball into their home", current_player + 1);
        if (is_win(board)) {
            printf(", but their holes are empty, the game is over\n");
            turn_state.game_outcome = GAME_OVER;
        } else {
            printf("! They get an extra turn.\n");
        }
    } else if (turn_outcome == INVALID) {
        printf("Something has gone TERRIBLY wrong!!!!!!\n");
        exit(EXIT_FAILURE);
    }
    return turn_state;
}

TurnState computer_turn(Board* board,
                        Player current_player,
                        uint8_t* p1_holes_sum,
                        uint8_t* p2_holes_sum,
                        uint32_t diff_level) {
    printf("Processing....\n");
    n_freed = 0;  // for stats
    StateNode* tree;
    tree = create_statenode(*board, current_player, diff_level);
    OptimalSolution* opt_sol = find_optimal_solution(tree, current_player);
    // should be enough
    char buf[300] = {0};
    write_strategy(opt_sol->strategy, opt_sol->idx, buf);
    printf("Computers solution: %s\n", buf);

    memcpy(board, &(opt_sol->statenode.board_state), sizeof(Board));

    free_optimal_solution(opt_sol);
    free_statenodes(tree);

    *p1_holes_sum = sum(board->p1_holes);
    *p2_holes_sum = sum(board->p2_holes);

    display_board(board);
    //printf("n_freed:%llu\n", n_freed);
    printf("Computer has finished its turn");

    TurnState turn_state;
    // the computer will never finish with a repeat
    turn_state.turn_outcome = COMPLETE;
    turn_state.game_outcome = GAME_NEXT;

    if (is_win(board)) {
        printf(", and the game is over.\n");
        turn_state.game_outcome = GAME_OVER;
    } else {
        printf("!\n");
    }
    return turn_state;
}

int print_and_return_results(Board* board, uint8_t p1_holes_sum, uint8_t p2_holes_sum, bool is_computer) {
    // The score is personal home + other player's holes
    uint8_t p1_score = board->p1_home + p2_holes_sum;
    uint8_t p2_score = board->p2_home + p1_holes_sum;

    // QUICK HACK! FIXME
    char p2_name[10];
    if (is_computer) {
        memcpy(p2_name, "Computer", 10);
    } else {
        memcpy(p2_name, "Player 2", 10);
    }

    printf("\n---RESULTS---\n");
    printf(
        "Player 1 has scored %d points (%d in home + %2d of %s's "
        "holes)\n",
        p1_score, board->p1_home, p2_holes_sum, p2_name);
    printf(
        "%s has scored %d points (%d in home + %2d of player 1's "
        "holes)\n",
        p2_name, p2_score, board->p2_home, p1_holes_sum);

    if (p1_score > p2_score) {
        printf("Player 1 wins! Thanks for playing the game.\n");
        return P1;
    } else if (p2_score > p1_score) {
        printf("%s wins! Thanks for playing the game.\n", p2_name);
        return P2;
    } else {
        printf("Draw! Thanks for playing the game.\n");
        return DRAW;
    }
}

// TODO: make it nicer, integrate with game_loop?
/*
    Main game loop (PvC). Returns the id of the winner `P1` or `P2`, or `DRAW`.
*/
int alt_game_loop(Board* board, uint32_t diff_level) {
    Player current_player = P1;
    display_board(board);
    uint8_t p1_holes_sum;
    uint8_t p2_holes_sum;
    TurnState game_result;
    for (;;) {
        if (current_player == P1) {
            game_result =
                player_turn(board, current_player, &p1_holes_sum, &p2_holes_sum);
        } else {
            game_result = computer_turn(board, current_player, &p1_holes_sum,
                                        &p2_holes_sum, diff_level);
        }
        if (game_result.game_outcome == GAME_OVER) {
            break;
        } else {
            if (game_result.turn_outcome == COMPLETE) {
                current_player = !current_player;
            }
            continue;
        }
    }
    return print_and_return_results(board, p1_holes_sum, p2_holes_sum, true);
}

// TODO: refactor to make it solver friendly (make another function?)
/*
    Main game loop (PvP). Returns the id of the winner `P1` or `P2`, or `DRAW`.
*/
int game_loop(Board* board) {
    Player current_player = P1;
    display_board(board);
    uint8_t p1_holes_sum;
    uint8_t p2_holes_sum;
    TurnState game_result;
    for (;;) {
        game_result = player_turn(board, current_player, &p1_holes_sum, &p2_holes_sum);
        if (game_result.game_outcome == GAME_OVER) {
            break;
        } else {
            if (game_result.turn_outcome == COMPLETE) {
                current_player = !current_player;
            }
            continue;
        }
    }
    return print_and_return_results(board, p1_holes_sum, p2_holes_sum, false);
}

int welcoming_prompt() {
    printf("---Welcome to Kalaha!---\n");
    printf("Choose mode:\n");
    printf(" 1) Two players\n");
    printf(" 2) Single player vs computer\n");

    for (;;) {
        char buffer[3];  // one digit, \n and \0
        char query[6];
        sprintf(query, ">>> ");
        int err = get_line(query, buffer, 4);
        if (err) {
            printf("Invalid input, try again!\n");
            continue;
        }
        int user_input;
        // includes EOF and failure to match (0)
        if (sscanf(buffer, "%d", &user_input) <= 0) {
            printf("Invalid input, try again!\n");
            continue;
        }
        if (user_input != 1 && user_input != 2) {
            printf("Invalid input, try again!\n");
            continue;
        }
        return user_input;
    }
}

uint32_t choose_difficulty() {
    printf("Choose difficulty level\n");
    for (;;) {
        char buffer[6];
        char query[6];
        sprintf(query, ">>> ");
        int err = get_line(query, buffer, 4);
        if (err) {
            printf("Invalid input, try again!\n");
            continue;
        }
        int user_input;
        // includes EOF and failure to match (0)
        if (sscanf(buffer, "%d", &user_input) <= 0) {
            printf("Invalid input, try again!\n");
            continue;
        }
        if (user_input <= 0) {
            printf("Invalid input, try again!\n");
            continue;
        }
        return user_input;
    }
}

int main() {
    Board _init_board = {
        {6, 6, 6, 6, 6, 6},  // player2's holes
        0,                   // player2's home
        {6, 6, 6, 6, 6, 6},  // player1's holes
        0                    // player1's home
    };
    Board* board = &_init_board;
    srand(time(NULL) + 1);

    int res = welcoming_prompt();
    if (res == PVP) {
        printf("Let's begin!\n\n");
        game_loop(board);
        return 0;
    }
    if (res == PVC) {
        uint32_t diff_level = choose_difficulty();
        printf("Let's begin!\n\n");
        alt_game_loop(board, diff_level);
    }

    return 0;
}
