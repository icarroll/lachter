#include <cassert>
#include <iostream>
#include <random>
#include <sstream>

#include "thudbit.hh"

const int NTRIES = 1000;

int main(int numargs, char * args[]) {
    bitboard board;
    for (int y=0 ; y<SIZE ; y+=1) {
        assert(board.data[y] == 0);
    }
    board[coord(5,5)] = true;
    assert(board.data[5] == LEFT_COLUMN_BIT >> 5);
    assert(board[coord(5,5)]);

    gamemove move1;
    assert(! move1.capt);
    stringstream move1_str("d K13-K11xK11");
    move1_str >> move1;
    assert(move1.capt);
    assert(move1.capts == 0b00000000);

    gamemove move2;
    assert(! move2.capt);
    stringstream move2_str("T H9-K11xK12xJ11");
    move2_str >> move2;
    assert(move2.capt);
    assert(move2.capts == 0b00010100);

    //TODO test all moves for validity

    mt19937 randgen(time(NULL));

    vector<double> scores = {};
    for (int tries=0 ; tries<NTRIES ; tries+=1) {
        gamestate state;
        if (! state.valid()) return 1;

        vector<gamemove> played = {};
        while (! state.gameover()) {
            vector<gamemove> allmoves = state.allmoves();
            uniform_int_distribution<> dist(0, allmoves.size()-1);
            int ix = dist(randgen);
            played.push_back(allmoves[ix]);
            state.domove(allmoves[ix]);
            if (! state.valid()) return 1;
        }
        scores.push_back(state.final_score());
    }

    double totalscore = 0;
    for (int ix=0 ; ix<scores.size() ; ix+=1) totalscore += scores[ix];
    double mean = totalscore / scores.size();

    vector<double> sqerrs = {};
    for (int ix=0 ; ix<scores.size() ; ix+=1) {
        double err = scores[ix] - mean;
        sqerrs.push_back(err * err);
    }
    double totalsqerr = 0;
    for (int ix=0 ; ix<sqerrs.size() ; ix+=1) totalsqerr += sqerrs[ix];
    double meansqerr = totalsqerr / sqerrs.size();

    cout << "score mean = " << mean << endl;
    cout << "score mean squared error = " << meansqerr << endl;

    return 0;
}
