#pragma once

#include <random>

#include "lachter.hh"

const float UCB1_C = 1.414;   // ~sqrt(2)

struct mcts_node {
    gamemove move_to_here;
    gamestate state;
    float win_total = 0.0;
    int visits = 0;
    vector<gamemove> unexplored_moves;
    vector<mcts_node> children = {};

    mcts_node(gamestate newstate);
    mcts_node(gamemove newmove, gamestate newstate);

    float sel_exp_sim_backprop(mt19937 randgen);
    float final_win();
    float simulate();
    float child_ucb(int ix);
};

struct mcts_brain {
    mt19937 randgen;
    mcts_node root;

    mcts_brain(gamestate newstate);

    void think_steps(int steps);
    gamemove best_move();
};
