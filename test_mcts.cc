#include <iostream>

#include "mcts.hh"

extern int max_depth;

int main(int numargs, char * args[]) {
    gamestate board;
    mcts_brain brain(board);

    brain.think_seconds(30.0);
    cout << "max depth searched: " << max_depth << endl;

    for (int ix=0 ; ix<brain.root.children.size() ; ix+=1) {
        cout << brain.root.children[ix].visits << " ";
    }
    cout << endl;

    int bestvisits = 0;
    int bestchild = -1;
    for (int ix=0 ; ix<brain.root.children.size() ; ix+=1) {
        if (brain.root.children[ix].visits > bestvisits) {
            bestvisits = brain.root.children[ix].visits;
            bestchild = ix;
        }
    }
    gamemove move = brain.root.children[bestchild].move_to_here;

    double win_total = brain.root.children[bestchild].win_total;
    cout << "chosen move: " << move << endl;
    cout << "move win_total: " << win_total << endl;
    cout << "move visits: " << bestvisits << endl;
    cout << "move win average: " << win_total / bestvisits << endl;
}
