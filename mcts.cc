#include <csignal>

#include "mcts.hh"

mcts_node::mcts_node(gamestate newstate) : move_to_here(), state(newstate) {
    unexplored_moves = state.allmoves();
}

mcts_node::mcts_node(gamemove newmove, gamestate newstate)
        : move_to_here(newmove), state(newstate) {
    unexplored_moves = state.allmoves();
}

int cur_depth;
int max_depth;

double mcts_node::sel_exp_sim_backprop(mt19937 randgen) {
    cur_depth += 1; if (cur_depth > max_depth) max_depth = cur_depth;

    visits += 1;

    if (state.gameover()) {
        // terminal node
        cur_depth -= 1;
        return final_win();
    }

    double win_guess;
    if (! unexplored_moves.empty()) {
        // leaf node -> expand/simulate
        uniform_int_distribution<> dist(0, unexplored_moves.size()-1);
        int ix = dist(randgen);
        gamemove move = unexplored_moves[ix];
        //TODO swap and pop_back instead
        unexplored_moves.erase(unexplored_moves.begin()+ix);

        gamestate newstate = state;
        newstate.domove(move);
        mcts_node newnode(move, newstate);

        win_guess = newnode.simulate(randgen);

        children.push_back(newnode);
    }
    else {
        // non-leaf node -> select
        double bestucb = -INFINITY;
        int bestchild = -1;
        for (int ix=0 ; ix<children.size() ; ix+=1) {
            double testucb = child_ucb(ix);
            if (testucb > bestucb) {
                bestucb = testucb;
                bestchild = ix;
            }
        }

        if (bestchild == -1) {
            cerr << "bad" << endl;
            raise(SIGSEGV);
        }

        win_guess = children[bestchild].sel_exp_sim_backprop(randgen);
    }

    // backpropagate
    win_total += win_guess;

    cur_depth -= 1;
    return win_guess;
}

double mcts_node::final_win() {
    // return value between 0.0 for pure dwarf win to 1.0 for pure troll win
    return (state.final_score() + MAX_DWARFS) / (2*MAX_DWARFS);
}

double mcts_node::simulate(mt19937 randgen) {
    // return value between 0.0 for pure dwarf win to 1.0 for pure troll win
    //double win_guess = (state.heuristic_score() + MAX_DWARFS) / (2*MAX_DWARFS);
    double win_guess = random_rollout(randgen);

    visits += 1;
    win_total += win_guess;

    return win_guess;
}

double mcts_node::random_rollout(mt19937 randgen) {
    // return value between 0.0 for pure dwarf win to 1.0 for pure troll win
    gamestate rollout_state = state;
    while (! rollout_state.gameover()) {
        vector<gamemove> allmoves = rollout_state.allmoves();
        uniform_int_distribution<> dist(0, allmoves.size()-1);
        int ix = dist(randgen);
        rollout_state.domove(allmoves[ix]);
    }

    double score = (rollout_state.final_score() + MAX_DWARFS) / (2*MAX_DWARFS);
    return score;
}

double mcts_node::child_ucb(int ix) {
    double win_total = children[ix].win_total;
    if (children[ix].move_to_here.isdwarfmove) win_total = 1 - win_total;
    return win_total / children[ix].visits
           + UCB1_C * sqrt(log(visits) / children[ix].visits);
}

mcts_node * mcts_node::best_child() {
    int bestvisits = 0;
    int bestchild = -1;
    for (int ix=0 ; ix<children.size() ; ix+=1) {
        if (children[ix].visits > bestvisits) {
            bestvisits = children[ix].visits;
            bestchild = ix;
        }
    }

    return & children[bestchild];
}

mcts_brain::mcts_brain(gamestate newstate) : root(newstate) {
    randgen = mt19937(time(NULL));
}

void mcts_brain::think_steps(int steps) {
    cur_depth = 0;
    max_depth = 0;
    for (int n=0 ; n<steps ; n+=1) root.sel_exp_sim_backprop(randgen);
}

void mcts_brain::think_seconds(double seconds) {
    cur_depth = 0;
    max_depth = 0;

    double start_time = get_now();
    double end_time = start_time + seconds;
    while (get_now() < end_time) {
        for (int n=0 ; n<STEPS ; n+=1) root.sel_exp_sim_backprop(randgen);
    }
}

double get_now() {
    struct timespec raw_time;
    clock_gettime(CLOCK_REALTIME_COARSE, & raw_time);

    return (double) raw_time.tv_sec + (double) raw_time.tv_nsec * 1e-9;
}

mcts_node * mcts_brain::best_child() {
    int bestvisits = 0;
    int bestchild = -1;
    for (int ix=0 ; ix<root.children.size() ; ix+=1) {
        if (root.children[ix].visits > bestvisits) {
            bestvisits = root.children[ix].visits;
            bestchild = ix;
        }
    }

    return & root.children[bestchild];
}

gamemove mcts_brain::best_move() {
    int bestvisits = 0;
    int bestchild = -1;
    for (int ix=0 ; ix<root.children.size() ; ix+=1) {
        if (root.children[ix].visits > bestvisits) {
            bestvisits = root.children[ix].visits;
            bestchild = ix;
        }
    }

    return root.children[bestchild].move_to_here;
}

void mcts_brain::do_move(gamemove move) {
    for (int ix=0 ; ix<root.children.size() ; ix+=1) {
        if (root.children[ix].move_to_here == move) {
            root = root.children[ix];
            break;
        }
    }
}
