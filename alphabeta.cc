#include "alphabeta.hh"

extern "C" {
#include "siphash.c"
}

// create invalid entry
alphabeta_entry::alphabeta_entry() {
}

// create valid defaults entry
alphabeta_entry::alphabeta_entry(uint64_t newhash) : hash(newhash) {
    set_checksum();
}

uint64_t zero[2] = {0,0};
uint64_t alphabeta_entry::compute_checksum() {
    uint64_t newchecksum;

    siphash((uint8_t *) & hash, (uint8_t *) & checksum - (uint8_t *) & hash,
            (uint8_t *) zero, (uint8_t *) & newchecksum, 8);

    return newchecksum;
}

bool alphabeta_entry::valid_checksum() {
    return checksum == compute_checksum();
}

void alphabeta_entry::set_checksum() {
    checksum = compute_checksum();
}

alphabeta_brain::alphabeta_brain(gamestate newstate) : state(newstate) {
    randgen = mt19937(time(NULL));
    ttable_size = TTABLE_SIZE;
    ttable = (alphabeta_entry *) malloc(ttable_size * sizeof(alphabeta_entry));
}

void alphabeta_brain::think_depth(int depth) {
    alphabeta(state, depth, -INFINITY, INFINITY, true);
}

double alphabeta_brain::alphabeta(gamestate node, int depth,
                                  double alpha, double beta, bool top) {
    if (node.gameover()) return node.final_score();
    if (depth == 0) return node.heuristic_score();

    alphabeta_entry entry = ttable_get(node);
    if (entry.depth >= depth) return entry.value;

    vector<gamemove> moves = node.allmoves();
    gamemove newbestmove;
    int numbestmoves = 0;

    bool cut = false;
    double value = node.isdwarfturn ? INFINITY : -INFINITY;
    for (gamemove move : moves) {
        gamestate newnode = node;
        newnode.domove(move);

        double newvalue = alphabeta(newnode, depth-1, alpha, beta);

        if (top && newvalue == value) {
            numbestmoves += 1;
            uniform_int_distribution<> dist(1,numbestmoves);
            if (dist(randgen) == 0) newbestmove = move;
        }
        else if (node.isdwarfturn) {
            if (newvalue < value) {
                value = newvalue;
                if (top) {newbestmove = move; numbestmoves = 1;}
            }
            if (value < beta) beta = value;
        }
        else {
            if (newvalue > value) {
                value = newvalue;
                if (top) {newbestmove = move; numbestmoves = 1;}
            }
            if (value > alpha) alpha = value;
        }

        if (alpha >= beta) {cut = true; break;}
    }

    entry.alpha = alpha;
    entry.beta = beta;
    entry.value = value;
    entry.depth = depth;
    ttable_put(entry);

    if (top) bestmove = newbestmove;
    return value;
}

gamemove alphabeta_brain::best_move() {
    //cout << "found " << bestmoves.size() << " moves" << endl;
    return bestmove;
}

alphabeta_entry alphabeta_brain::ttable_get(gamestate state) {
    alphabeta_entry entry = ttable[state.hash % ttable_size];
    if (entry.hash == state.hash && entry.valid_checksum()) return entry;
    else return alphabeta_entry(state.hash);
}

void alphabeta_brain::ttable_put(alphabeta_entry entry) {
    entry.set_checksum();
    ttable[entry.hash % ttable_size] = entry;
}
