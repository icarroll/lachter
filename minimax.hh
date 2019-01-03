#pragma once

#include <climits>
#include <memory>
#include <random>

extern "C" {
#include <time.h>
}

#include "thud1.hh"

struct minimax_brain {
    gamestate state;
    vector<gamemove> bestmoves = {};
    //mt19937 randgen;

    minimax_brain(gamestate newstate);

    void think_depth(int depth);
    //void think_seconds(double seconds);
    double minimax(gamestate node, int depth, bool top=false);

    gamemove best_move();

    void do_move(gamemove move);
};

