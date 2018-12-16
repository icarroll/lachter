#include <iostream>
#include <random>

#include "lachter.hh"

int main(int numargs, char * args[]) {
    mt19937 gen(0);

    for (int tries=0 ; tries<1000 ; tries+=1) {
        gamestate board;
        if (! board.valid()) return 1;

        vector<gamemove> played = {};
        while (! board.gameover()) {
            vector<gamemove> allmoves = board.allmoves();
            uniform_int_distribution<> dis(0, allmoves.size());
            int ix = dis(gen);
            played.push_back(allmoves[ix]);
            board.domove(allmoves[ix]);
            if (! board.valid()) return 1;
        }
    }

    return 0;
}
