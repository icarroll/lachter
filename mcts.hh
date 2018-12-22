#pragma once

#include <random>

extern "C" {
#include <time.h>
}

#include "lachter.hh"

//const double UCB1_C = 1.414;   // ~sqrt(2)
//const double UCB1_C = 1.0e-4; //TODO tune this by hill climbing
const double UCB1_C = 1.0e-5;

struct mcts_node {
    gamemove move_to_here;
    gamestate state;
    double win_total = 0.0;
    int visits = 0;
    vector<gamemove> unexplored_moves;
    vector<mcts_node> children = {};

    mcts_node(gamestate newstate);
    mcts_node(gamemove newmove, gamestate newstate);

    double sel_exp_sim_backprop(mt19937 randgen);
    double final_win();
    double simulate();
    double child_ucb(int ix);

    mcts_node best_child();
};

const int STEPS = 1000;   // less than .1sec on a fast machine

struct mcts_brain {
    mt19937 randgen;
    mcts_node root;

    mcts_brain(gamestate newstate);

    void think_steps(int steps);
    void think_seconds(double seconds);
    mcts_node best_child();
    gamemove best_move();
};

double get_now();
