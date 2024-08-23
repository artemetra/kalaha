#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

StateNode grow_statetree(Board board) {
    // uint8_t holes[6] = board.p2_holes;
}

// TODO: get rid of all of this shit and write a version of a minimax algorithm

StateNode* minimax(Board board, uint32_t depth) {}

StateNode* create_statenode(Board board, uint32_t level) {
    // printf("%u\n", level);
    // if (level > 10) { return NULL; }
    //  using calloc here will set is_last field to false
    //  and all of the pointers to NULL
    StateNode* statenode = (StateNode*)calloc(1, sizeof(StateNode));
    memcpy(statenode, &board, sizeof(Board));

    for (uint8_t i = 0; i < 6; i++) {
        if (board.p2_holes[i] > 0) {
            Board* new_board = (Board*)malloc(sizeof(Board));
            memcpy(new_board, &board, sizeof(Board));

            TurnOutcome outcome = make_a_turn(new_board, i, 1);
            // print_outcome(outcome);
            printf("br\n");
            if (outcome == COMPLETE) {
                StateNode* sub_statenode = (StateNode*)calloc(1, sizeof(StateNode));
                sub_statenode->is_last = true;
                sub_statenode->board_state = *new_board;
                statenode->paths[i] = sub_statenode;

            } else if (outcome == REPEAT) {
                statenode->paths[i] = create_statenode(*new_board, level + 1);
            } else {
                printf("wut?");
            }
        }
    }

    return statenode;
}

// TODO
void free_statenode(StateNode* statenode) {}

// output needs to be `nholes*sizeof(TurnOutcome)`
void try_plays(
    Board* board,
    Player player_id,
    uint8_t* holes,
    uint8_t nholes,
    TurnOutcome* output)
{
    for (size_t i=0; i<nholes; i++) {
        Board board_copy;
        memcpy(&board_copy, board, sizeof(Board));
        int idx = convert_index(holes[i], player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);
        output[i] = out;
    }
}
