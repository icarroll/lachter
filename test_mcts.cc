#include <iomanip>
#include <iostream>

#include "mcts.hh"

extern int max_depth;

int main(int numargs, char * args[]) {
    gamestate board;
    mcts_brain brain(board);

    brain.think_seconds(0.1);
    //cout << "max depth searched: " << max_depth << endl;

    for (int ix=0 ; ix<brain.root->children.size() ; ix+=1) {
        //cout << brain.root->children[ix]->visits << " ";
    }
    //cout << endl;

    int bestvisits = 0;
    int bestchild = -1;
    for (int ix=0 ; ix<brain.root->children.size() ; ix+=1) {
        if (brain.root->children[ix]->visits > bestvisits) {
            bestvisits = brain.root->children[ix]->visits;
            bestchild = ix;
        }
    }
    shared_ptr<mcts_node> best_child = brain.root->best_child();
    gamemove move = best_child->move_to_here;

    /*
    cout << fixed << setprecision(3);
    cout << "chosen move: " << move << endl;
    cout << "move win_total: " << best_child->win_total << endl;
    cout << "move visits: " << bestvisits << endl;
    cout << "move win average: " << best_child->win_total / bestvisits << endl;

    cout << "principal variation:" << endl;
    */

    shared_ptr<mcts_node> node = brain.root;
    do {
        node = node->best_child();
        //cout << node->move_to_here << " : " << node->win_total << '/' << node->visits << '=' << (node->win_total / node->visits) << endl;
    } while (node->children.size() > 0);

    return 0;
}
