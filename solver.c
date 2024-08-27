#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

uint32_t MAX_LEVEL = 75;
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
        root->is_last = true;
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
        if (out != INVALID) {
            root->paths[i - 1] = child;
        } else {
            free(child);
        }
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

void update_opt_sol(OptimalSolution* opt_sol,
                    StateNode* node,
                    uint8_t strategy[MAX_STRAT_LEN],
                    uint8_t idx) {
    if (!opt_sol->statenode.is_last) {
        memcpy(&opt_sol->statenode, node, sizeof(StateNode));
        memcpy(opt_sol->strategy, strategy, sizeof(uint8_t) * MAX_STRAT_LEN);
        opt_sol->idx = idx;
        return;
    }

    uint8_t curr;
    uint8_t upd;
    if (opt_sol->player_id == P1) {
        curr = opt_sol->statenode.board_state.p1_home;
        upd = node->board_state.p1_home;
    }
    if (opt_sol->player_id == P2) {
        curr = opt_sol->statenode.board_state.p2_home;
        upd = node->board_state.p2_home;
    }

    if (upd > curr) {
        memcpy(&opt_sol->statenode, node, sizeof(StateNode));
        memcpy(opt_sol->strategy, strategy, sizeof(uint8_t) * MAX_STRAT_LEN);
        opt_sol->idx = idx;
    }
}

// Postorder traversal
// opt_sol is output
void traverse_tree(StateNode* node,
                   uint8_t strategy[MAX_STRAT_LEN],
                   uint8_t idx,
                   OptimalSolution* opt_sol) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < 6; i++) {
        strategy[idx] = i + 1;
        traverse_tree(node->paths[i], strategy, idx + 1, opt_sol);
    }
    if (node->is_last) {
        update_opt_sol(opt_sol, node, strategy, idx);
    }
}

// Writes the strategy to a string `buf`
void write_strategy(uint8_t strategy[MAX_STRAT_LEN],
                    uint8_t idx,
                    char* buf,
                    size_t size) {
    if (idx == 0) {
        return;
    }
    char temp[5];
    snprintf(temp, 5, "%d", strategy[0]);
    strcat(buf, temp);
    for (size_t i = 1; i <= idx; i++) {
        snprintf(temp, 5, "->%d", strategy[i]);
        strcat(buf, temp);
    }
}