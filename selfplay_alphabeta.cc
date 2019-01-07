#include <iostream>

#include "alphabeta.hh"

const int THINK_DEPTH = 4;

int main(int numargs, char * args[]) {
    gamestate board;
    alphabeta_brain brain(board);

    while (! brain.state.gameover()) {
        brain.think_depth(THINK_DEPTH);
        gamemove move = brain.best_move();
        cout << move << endl;
        brain.do_move(move);
    }
}
