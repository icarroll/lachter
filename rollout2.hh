#pragma once

#include <cstring>
#include <map>
#include <memory>
#include <set>

extern "C" {
#include <time.h>
}

#include "thudbit.hh"

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
    bool valid_checksum();
    void set_checksum();
};

// default to largest prime less than 100 million
// results in table size of ~4.5GB
const int TTABLE_SIZE = 99999787;

const double C = 1.414;   // try sqrt(2) for old-time's sake
//const int STEPS = 1000;   // less than .1sec on a fast machine
const int STEPS = 1;

struct rollout2_brain {
    mt19937 randgen;
    gamestate state;

    uint64_t ttable_size;
    rollout2_entry * ttable;

    rollout2_brain(gamestate newstate);

    void think_seconds(double seconds);
    double rollout2(gamestate s, double alpha_s, double beta_s);

    gamemove best_move();

    void do_move(gamemove move);

    rollout2_entry ttable_get(gamestate state);
    void ttable_put(rollout2_entry entry);
};

double get_now();
