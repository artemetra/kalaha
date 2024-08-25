#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

const uint32_t MAX_LEVEL = 100;
uint64_t possible_final_board_states = 0;
// Returns a fully grown statenode from a board and a player_id.
StateNode* create_statenode(Board board, Player player_id) {
    possible_final_board_states = 0;
    StateNode* root = malloc(sizeof(StateNode));
    *root = (StateNode){
        board,
        false,
        {NULL},
    };
    grow_statenodes(root, player_id, 0);
    return root;
}

// Recursively grow statenodes from the root.
void grow_statenodes(StateNode* root, Player player_id, uint32_t level) {
    if (level >= MAX_LEVEL) {
        return;
    }
    bool are_all_complete = true;
    for (int i = 1; i <= 6; i++) {
        Board board_copy;
        memcpy(&board_copy, &root->board_state, sizeof(Board));

        int idx = convert_index(i, player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);

        StateNode* child = malloc(sizeof(StateNode));
        if (!child) {
            exit(EXIT_FAILURE);
        }
        *child = (StateNode){.board_state = board_copy, .is_last = true, .paths = {NULL}};

        if (out == REPEAT) {
            child->is_last = false;
            are_all_complete = false;
            grow_statenodes(child, player_id, level + 1);
        }
        root->paths[i - 1] = child;
    }
    if (are_all_complete) {
        possible_final_board_states++;
    }
}

// Recursively frees all statenodes starting from `node` AND `node` itself.
void free_statenodes(StateNode* node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < 6; i++) {
        if (node->paths[i] != NULL) {
            free_statenodes(node->paths[i]);
        }
    }
    free(node);
}
