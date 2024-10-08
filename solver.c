#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"

uint64_t n_freed;

StateNode* create_statenode(Board board, Player player_id, const uint32_t max_level) {
    StateNode* root = malloc(sizeof(StateNode));
    *root = (StateNode){
        board,
        false,
        {NULL},
    };
    grow_statenodes(root, player_id, max_level);
    return root;
}

bool are_all_zero(uint8_t arr[6]) {
    bool all_zero = true;
    for (size_t i = 0; i < 6; i++) {
        if (arr[i]) {
            all_zero = false;
        }
    }
    return all_zero;
}

void grow_statenodes(StateNode* root, Player player_id, uint32_t level) {
    if (level == 0) {
        root->is_last = true;
        return;
    }
    for (int i = 1; i <= 6; i++) {
        Board board_copy;
        memcpy(&board_copy, &root->board_state, sizeof(Board));

        int idx = convert_index(i, player_id);
        TurnOutcome out = make_a_turn(&board_copy, idx, player_id);
        if (out == INVALID) {
            continue;
        }
        StateNode* child = malloc(sizeof(StateNode));
        if (!child) {
            exit(EXIT_FAILURE);
        }
        *child = (StateNode){.board_state = board_copy, .is_last = true, .paths = {NULL}};

        if (out == REPEAT) {
            uint8_t(*pits)[6];
            if (player_id == P1) {
                pits = &board_copy.p1_pits;
            } else {
                pits = &board_copy.p2_pits;
            }
            if (!are_all_zero(*pits)) {
                child->is_last = false;
                grow_statenodes(child, player_id, level - 1);
            }
        }
        root->paths[i - 1] = child;
    }
}

void free_statenodes(StateNode* node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < 6; i++) {
        if (node->paths[i] != NULL) {
            free_statenodes(node->paths[i]);
        }
    }
    n_freed += sizeof(StateNode);
    free(node);
}

void update_opt_sol(OptimalSolution* opt_sol,
                    StateNode* node,
                    uint8_t strategy[MAX_STRAT_LEN],
                    uint8_t idx) {
    // If opt_sol is "empty", set it any value
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

    // If the new state has more beads in the home, update
    if (upd > curr) {
        memcpy(&opt_sol->statenode, node, sizeof(StateNode));
        memcpy(opt_sol->strategy, strategy, sizeof(uint8_t) * MAX_STRAT_LEN);
        opt_sol->idx = idx;
        return;
    }

    // If the new state has the same solution but in less steps, update
    if (upd == curr) {
        if (idx < opt_sol->idx && idx != 0) {
            memcpy(&opt_sol->statenode, node, sizeof(StateNode));
            memcpy(opt_sol->strategy, strategy, sizeof(uint8_t) * MAX_STRAT_LEN);
            opt_sol->idx = idx;
        }
    }
}

/*
    Postorder traversal of the statenode tree, calling `update_opt_sol` on every found
    "last node".
*/
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
        if (strategy[idx] == 6 && node->paths[5] == NULL) {
            strategy[idx] = 0;
            idx--;
        }
        update_opt_sol(opt_sol, node, strategy, idx);
    }
}

OptimalSolution* find_optimal_solution(StateNode* root, Player player_id) {
    uint8_t strategy[MAX_STRAT_LEN] = {0};

    OptimalSolution* opt_sol = malloc(sizeof(OptimalSolution));
    *opt_sol = (OptimalSolution){
        .player_id = player_id,
        .idx = 0,
        .strategy = {0},
        .statenode = *root,
    };

    traverse_tree(root, strategy, 0, opt_sol);

    return opt_sol;
}

void free_optimal_solution(OptimalSolution* opt_sol) {
    free(opt_sol);
}

void write_strategy(uint8_t strategy[MAX_STRAT_LEN], uint8_t idx, char* buf) {
    char temp[5];
    snprintf(temp, 5, "%d", strategy[0]);
    strcat(buf, temp);
    for (size_t i = 1; i <= idx; i++) {
        snprintf(temp, 5, "->%d", strategy[i]);
        strcat(buf, temp);
    }
}