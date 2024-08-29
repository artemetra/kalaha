#ifndef KALAHA_SOLVER_H_
#define KALAHA_SOLVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "game.h"

#define MAX_STRAT_LEN 100

/*
    Represents a node of a tree of board states.

    `board_state` -- copy of the board state.
    `is_last` -- true if the node is last. Should be used instead of checking whether
    every path in `paths` is `NULL`.
    `paths` -- array of 6 pointers to other `StateNode`s, each individually can be `NULL`.
    Represents the state of the board after picking the hole `idx+1` where `idx` is the
    index used with the array.
*/
typedef struct StateNode {
    Board board_state;
    bool is_last;
    struct StateNode* paths[6];
} StateNode;

/*
    Returns a fully "grown" statenode tree from a `board` and a `player_id`.
    The function will explore all possible plays `player_id` can do, including additional
    turns, up to `max_level` of depth. All statenodes are allocated on the heap, call
    `free_statenodes` after use.
*/
StateNode* create_statenode(Board board, Player player_id, const uint32_t max_level);

/*
    Recursively "grows" statenodes from a root, up to `level`. Should not be used
    directly, instead call `create_statenode`.
*/
void grow_statenodes(StateNode* root, Player player_id, uint32_t level);

/*
    Recursively frees the statenode tree from `node` AND frees the `node` itself.
*/
void free_statenodes(StateNode* node);

/*
    Helper struct for the optimal solution found from `find_optimal_solution`.

    `statenode` -- resulting statenode from applying the solution. The resulting board
    state can then just be taken from this.
    `strategy` -- an array representing the sequence of holes picked to get to the final
    state. The real length of the array is `idx+1`. The unused array space is guaranteed
    to be {0}.
    `idx` -- index to the last element of `strategy`.
    `player_id` -- player id, for whom the optimal solution is found.
*/
typedef struct OptimalSolution {
    StateNode statenode;
    uint8_t strategy[MAX_STRAT_LEN];
    uint8_t idx;
    Player player_id;
} OptimalSolution;

/*
    Returns a pointer to an optimal solution for a given tree for a player.
    The solution is found by traversing the statenode tree and looking for all possible
    end states and finding the one that has the maximum count of balls in `player_id`s
    home. It does not consider how (possibly) beneficial is the final board state for the
    other player could be. The solution is not guaranteed to be the shortest (TODO?)

    `root` (IN) -- root statenode
    `player_id` -- player_id
*/
OptimalSolution* find_optimal_solution(StateNode* root, Player player_id);
/*
    Frees the OptimalSolution pointer.
    Not sure if this is even needed lol

    `opt_sol` (IN/OUT)
*/
void free_optimal_solution(OptimalSolution* opt_sol);

/*
    Writes the solving strategy to a buffer.
    Example: "1->5->2->6".

    `strategy` -- the strategy to print.
    `idx` -- index to the last strategy element.
    `buf` (OUT) -- output buffer
*/
void write_strategy(uint8_t strategy[MAX_STRAT_LEN], uint8_t idx, char* buf);

#endif  // KALAHA_SOLVER_H_
