#include <iomanip>
#include <iostream>

#include "rollout2.hh"

extern int max_depth;

int main(int numargs, char * args[]) {
    gamestate board;
    rollout2_brain brain(board);

    brain.think_seconds(0.1);
    gamemove move = brain.best_move();
    brain.do_move(move);

    return 0;
}
