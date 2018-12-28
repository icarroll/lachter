#include "minimax.hh"

minimax_brain::minimax_brain(gamestate newstate) : state(newstate) {
}

void minimax_brain::think_depth(int depth) {
    minimax(state, depth, true);
}

double minimax_brain::minimax(gamestate node, int depth, bool top) {
    if (node.gameover()) return node.final_score();
    if (depth == 0) return node.heuristic_score();

    vector<gamemove> moves = node.allmoves();

    int value = node.isdwarfturn ? INT_MAX : INT_MIN;
    for (gamemove move : moves) {
        gamestate newnode = node;
        newnode.domove(move);
        int newvalue = minimax(newnode, depth-1);
        if (top && newvalue == value) {
            bestmoves.push_back(move);
        }
        else if (node.isdwarfturn) {
            if (newvalue < value) {
                value = newvalue;
                if (top) bestmoves = {move};
            }
        }
        else {
            if (newvalue > value) {
                value = newvalue;
                if (top) bestmoves = {move};
            }
        }
    }

    return value;
}

gamemove minimax_brain::best_move() {
    //cout << "found " << bestmoves.size() << " moves" << endl;
    return bestmoves[0];
}
