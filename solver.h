#ifndef KALAHA_SOLVER_H_
#define KALAHA_SOLVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game.h"

extern uint64_t possible_final_board_states;
#define MAX_STRAT_LEN 100

/*
        Represents a node of a tree of board states.
        Each node has 6 connections (pointers) which can be null.
        is_last is true if the turn is complete.
*/
typedef struct StateNode {
    Board board_state;
    bool is_last;
    struct StateNode* paths[6];
} StateNode;

StateNode* create_statenode(Board board, Player player_id);
void grow_statenodes(StateNode* root, Player player_id, uint32_t level);
void free_statenodes(StateNode* node);

typedef struct OptimalSolution {
    StateNode statenode;
    uint8_t strategy[MAX_STRAT_LEN];
    uint8_t idx;
    Player player_id;
} OptimalSolution;

void traverse_tree(StateNode* node,
                   uint8_t strategy[MAX_STRAT_LEN],
                   uint8_t idx,
                   OptimalSolution* optimal_solution);

void print_strategy(uint8_t strategy[MAX_STRAT_LEN], uint8_t idx);

void try_plays(Board* board,
               Player player_id,
               uint8_t* holes,
               uint8_t nholes,
               TurnOutcome* output);

#endif  // KALAHA_SOLVER_H_
