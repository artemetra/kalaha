#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

const uint32_t MAX_LEVEL = 100;

StateNode create_statenode(Board board, Player player_id) {
    StateNode root = {
        board,
        {NULL},
    };
    grow_statenodes(&root, player_id, 0);
}

void grow_statenodes(StateNode* root, Player player_id, uint32_t level) {
    if (level >= MAX_LEVEL) {
        return;
    }
    for (int i = 1; i <= 6; i++) {
        // Create a deep copy of the board state
        Board board_copy;
        memcpy(&board_copy, &root->board_state, sizeof(Board));
        
        // Convert move and make a turn
        int idx = convert_index(i, player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);

        // Allocate and initialize the child node
        StateNode* child = malloc(sizeof(StateNode));
        if (!child) {
            // Handle memory allocation failure if necessary
            exit(EXIT_FAILURE);
        }
        *child = (StateNode){
            .board_state = board_copy,
            .paths = {NULL}  // Initialize paths to NULL
        };

        // Recursive call based on the outcome
        if (out == REPEAT) {
            grow_statenodes(child, player_id, level + 1);
        }

        // Store the child node in the paths array
        root->paths[i - 1] = child;
    }
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
