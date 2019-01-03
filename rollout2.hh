#pragma once

#include <map>
#include <memory>
#include <random>
#include <set>
#include <tuple>

extern "C" {
#include <time.h>
}

#include "lachter.hh"

struct rollout2_node {
    gamemove move_to_here;
    gamestate state;
    double win_total = 0.0;
    int visits = 0;
    vector<gamemove> unexplored_moves;
    vector<shared_ptr<rollout2_node>> children = {};

    rollout2_node(gamestate newstate);
    rollout2_node(gamemove newmove, gamestate newstate);

    double sel_exp_sim_backprop(mt19937 randgen);
    double final_win();
    double simulate(mt19937 randgen);
    double random_rollout(mt19937 randgen);
    double child_ucb(int ix);

    //shared_ptr<rollout2_node> best_child();
};

struct rollout2_entry {
    uint64_t hash;
    int visits = 0;
    double mu = 0;
    double vminus = -INFINITY;
    double vplus = INFINITY;
    uint64_t checksum;

    rollout2_entry();
    rollout2_entry(uint64_t newhash);

    uint64_t compute_checksum();
};

// default to largest prime less than 100 million
// results in table size of ~4.5GB
const int TTABLE_SIZE = 99999787;

const double C = 1.414;   // try sqrt(2) for old-time's sake

struct rollout2_brain {
    mt19937 randgen;
    gamestate state;

    vector<rollout2_entry> ttable = {};

    rollout2_brain(gamestate newstate);

    void think_seconds(double seconds);
    double rollout2(gamestate s, double alpha_s, double beta_s);

    gamemove best_move();

    void do_move(gamemove move);

    rollout2_entry ttable_get(gamestate state);
    void ttable_put(rollout2_entry entry);
};
