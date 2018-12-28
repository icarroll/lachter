#include <iomanip>
#include <iostream>

#include "minimax.hh"

int main(int numargs, char * args[]) {
    gamestate board;
    minimax_brain brain(board);

    brain.think_depth(2);
    //cout << brain.best_move() << endl;

    return 0;
}
