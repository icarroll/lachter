#include <iostream>
#include <random>

#include "thudbit.hh"

const int NTRIES = 1000;

int main(int numargs, char * args[]) {
    gamemove check(false, coord(6,5), coord(6,1), true, 0b11000000);
    if (! check.capt) return 1;

    mt19937 randgen(time(NULL));

    vector<float> scores = {};
    for (int tries=0 ; tries<NTRIES ; tries+=1) {
        gamestate board;
        if (! board.valid()) return 1;

        vector<gamemove> played = {};
        while (! board.gameover()) {
            vector<gamemove> allmoves = board.allmoves();
            uniform_int_distribution<> dist(0, allmoves.size()-1);
            int ix = dist(randgen);
            played.push_back(allmoves[ix]);
            board.domove(allmoves[ix]);
            if (! board.valid()) return 1;
        }
        scores.push_back(board.final_score());
    }

    float totalscore = 0;
    for (int ix=0 ; ix<scores.size() ; ix+=1) totalscore += scores[ix];
    float mean = totalscore / scores.size();

    vector<float> sqerrs = {};
    for (int ix=0 ; ix<scores.size() ; ix+=1) {
        float err = scores[ix] - mean;
        sqerrs.push_back(err * err);
    }
    float totalsqerr = 0;
    for (int ix=0 ; ix<sqerrs.size() ; ix+=1) totalsqerr += sqerrs[ix];
    float meansqerr = totalsqerr / sqerrs.size();

    cout << "score mean = " << mean << endl;
    cout << "score mean squared error = " << meansqerr << endl;

    return 0;
}
