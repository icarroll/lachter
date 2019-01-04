#include <iomanip>
#include <iostream>

#include "rollout2.hh"

extern int max_depth;

int main(int numargs, char * args[]) {
    gamestate board;
    rollout2_brain brain(board);

    cout << "starting think" << endl;
    brain.think_seconds(60.0);
    cout << "thinking done" << endl;
    gamemove move = brain.best_move();
    //brain.do_move(move);

    return 0;
}
