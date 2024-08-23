#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

void grow_statetree(Board board) {
    // uint8_t holes[6] = board.p2_holes;
}

// TODO: get rid of all of this shit and write a version of a minimax algorithm

void minimax(Board board, uint32_t depth) {}

StateNode create_statenode(Board board, Player player_id, uint32_t level) {
    uint8_t init_home;
    if (player_id == P1) {
        init_home = board.p1_home;
    }
    if (player_id == P2) {
        init_home = board.p2_home;
    }

    StateNode* paths[6] = {NULL};
    StateNode root = {
        board,
        false,
        paths,
    };
    for (size_t i = 1; i <= 6; i++) {
        Board board_copy;
        memcpy(&board_copy, &board, sizeof(Board));
        int idx = convert_index(i, player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);
        if (player_id == P1) {
            uint8_t bruh = board_copy.p1_home - init_home;
            printf("%d", bruh);
        }
        if (player_id == P2) {
            printf("%d", board_copy.p2_home - init_home);
        }
        if (out == COMPLETE) {
            printf("C ");
        } else if (out == REPEAT) {
            printf("R ");
        }
    }
    printf("\n");
    return root;
}

// TODO
void free_statenode(StateNode* statenode) {}

// output needs to be `nholes*sizeof(TurnOutcome)`
void try_plays(Board* board,
               Player player_id,
               uint8_t* holes,
               uint8_t nholes,
               TurnOutcome* output) {
    for (size_t i = 0; i < nholes; i++) {
        Board board_copy;
        memcpy(&board_copy, board, sizeof(Board));
        int idx = convert_index(holes[i], player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);
        output[i] = out;
    }
}
