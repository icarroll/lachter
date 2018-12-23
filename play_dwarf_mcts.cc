#include <iostream>

#include "mcts.hh"

const double THINK_TIME = 60.0;

int main(int numargs, char * args[]) {
    gamestate board;
    mcts_brain brain(board);

    brain.think_seconds(THINK_TIME);
    gamemove move = brain.best_move();
    cout << move << endl;
    brain.do_move(move);

    gamemove yourmove;
    cin >> yourmove;
    brain.do_move(yourmove);
}
