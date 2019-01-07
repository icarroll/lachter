#include <iomanip>
#include <iostream>

#include "alphabeta.hh"

int main(int numargs, char * args[]) {
    gamestate board;
    alphabeta_brain brain(board);

    brain.think_depth(4);
    //cout << brain.best_move() << endl;

    return 0;
}
