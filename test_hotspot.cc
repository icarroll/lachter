#include <iomanip>
#include <iostream>

#include "alphabeta.hh"

int main(int numargs, char * args[]) {
    gamestate board;
    alphabeta_brain brain(board);

    brain.hotspot_heuristic_evaluation(board);

    return 0;
}
