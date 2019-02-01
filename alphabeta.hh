#pragma once

#include <climits>
#include <functional>
#include <memory>
#include <random>

extern "C" {
#include <time.h>
}

#include "thudbit.hh"

struct alphabeta_entry {
    uint64_t hash;
    double alpha = -INFINITY;
    double beta = INFINITY;
    double value = 0;
    int depth = 0;
    uint64_t checksum;

    alphabeta_entry();
    alphabeta_entry(uint64_t newhash);

    uint64_t compute_checksum();
    bool valid_checksum();
    void set_checksum();
};

// default to largest prime less than 100 million
// results in table size of ~4.5GB
const int TTABLE_SIZE = 99999787;

struct hotspot {
    bitboard mask = {};

    hotspot(gamemove move);

    bool intersects(hotspot & that);
    void add(hotspot & that);
};

vector<hotspot> merge(vector<hotspot> hotspots);
vector<hotspot> merge1(hotspot & newhotspot, vector<hotspot> hotspots);

struct alphabeta_brain {
    mt19937 randgen;
    gamestate state;
    gamemove bestmove;

    int current_base_search_depth;
    bool ok_to_extend = true;

    uint64_t ttable_size;
    alphabeta_entry * ttable;

    function<double(gamestate)> evaluate;

    alphabeta_brain(gamestate newstate);

    //void think_seconds(double seconds);
    void iterative_deepen(int maxdepth);
    void think_depth(int depth);
    double alphabeta(gamestate node, int depth, double alpha, double beta,
                     bool top=false);
    double hotspot_heuristic_evaluation(gamestate node);

    gamemove best_move();

    void do_move(gamemove move);

    alphabeta_entry ttable_get(gamestate state);
    void ttable_put(alphabeta_entry entry);
};
