#include <iostream>

#include "mcts.hh"

int main(int numargs, char * args[]) {
    gamestate board;
    mcts_brain brain(board);

    brain.think_steps(10000);
    brain.best_move();
}
