#pragma once

#include "lachter.hh"

struct mcts_brain {
    gamestate * state;

    mcts_brain(gamestate * newstate);

    void think_steps(int steps);
    void think_step();
};
