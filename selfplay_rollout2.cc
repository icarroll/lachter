#include <iostream>

#include "rollout2.hh"

const double THINK_TIME = 10.0;

int main(int numargs, char * args[]) {
    gamestate board;
    rollout2_brain brain(board);

    while (! brain.state.gameover()) {
        brain.think_seconds(THINK_TIME);
        gamemove move = brain.best_move();
        cout << move << endl;
        brain.do_move(move);
    }
}
