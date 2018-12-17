#include <random>

#include "mcts.hh"

mcts_brain::mcts_brain(gamestate * newstate) : state(newstate) {
}

void mcts_brain::think_steps(int steps) {
    for (int n=0 ; n<steps ; n+=1) think_step();
}

void mcts_brain::think_step() {
}
