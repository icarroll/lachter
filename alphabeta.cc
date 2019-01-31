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

/*
    ###
    #S##
    ##\##
     ##E#
      ###
*/
hotspot::hotspot(gamemove move) {
    int topy, boty;
    int leftx, rightx;
    uint16_t line;
    if (move.from.y == move.to.y) {
        // horizontal
        topy = min(0, move.from.y - 1);
        boty = max(SIZE, move.from.y + 1);
        leftx = min(0, min(move.from.x, move.to.x) - 1);
        rightx = max(SIZE, max(move.from.x, move.to.x) + 1);
        line = VALID_BITS
               & ((LEFT_COLUMN_BIT >> (leftx-1)) - 1)
               & ~ ((LEFT_COLUMN_BIT >> (rightx-1)) - 1);
        for (int y=topy ; y<=boty ; y+=1) mask.data[y] |= line;
    }
    else if (move.from.x == move.to.x) {
        // vertical
        topy = min(0, min(move.from.y, move.to.y) - 1);
        boty = max(SIZE, max(move.from.y, move.to.y) + 1);
        leftx = min(0, move.from.x - 1);
        rightx = max(SIZE, move.from.x + 1);
        line = VALID_BITS
               & ((LEFT_COLUMN_BIT >> (leftx-1)) - 1)
               & ~ ((LEFT_COLUMN_BIT >> (rightx-1)) - 1);
        for (int y=topy ; y<=boty ; y+=1) mask.data[y] |= line;
    }
    else {
        topy = min(0, min(move.from.y, move.to.y) - 1);
        boty = max(SIZE, max(move.from.y, move.to.y) + 1);
        leftx = min(0, min(move.from.x, move.to.x) - 1);
        rightx = max(SIZE, max(move.from.x, move.to.x) + 1);
        uint16_t fiveones = 0b1111100000000000;
        uint16_t col_mask = VALID_BITS
                            & ((LEFT_COLUMN_BIT >> (leftx-1)) - 1)
                            & ~ ((LEFT_COLUMN_BIT >> (rightx-1)) - 1);
        int x, dx;
        if ((move.to.y - move.from.y) / (move.to.x - move.from.x) > 0) {
            x = leftx;
            dx = +1;
        }
        else {
            x = rightx;
            dx = -1;
        }
        for (int y=topy ; y<=boty ; y+=1) {
            line = col_mask & (fiveones >> (x-1));
            mask.data[y] |= line;
            x += dx;
        }
    }
}

vector<hotspot> merge(vector<hotspot> hotspots) {
    vector<hotspot> merged = {};
    for (hotspot & cur : hotspots) {
        //TODO
    }

    return merged;
}

alphabeta_brain::alphabeta_brain(gamestate newstate) : state(newstate) {
    randgen = mt19937(time(NULL));
    ttable_size = TTABLE_SIZE;
    ttable = (alphabeta_entry *) malloc(ttable_size * sizeof(alphabeta_entry));
    evaluate = [](gamestate node) {
        return node.heuristic_score();
    };
}

void alphabeta_brain::iterative_deepen(int maxdepth) {
    for (int depth=1 ; depth<=maxdepth ; depth+=1) think_depth(depth);
}

void alphabeta_brain::think_depth(int depth) {
    alphabeta(state, depth, -INFINITY, INFINITY, true);
}

double alphabeta_brain::alphabeta(gamestate node, int depth,
                                  double alpha, double beta, bool top) {
    if (top) current_base_search_depth = depth;

    if (node.gameover()) return node.final_score();
    if (depth == 0) return evaluate(node);

    alphabeta_entry entry = ttable_get(node);
    if (entry.depth >= depth) return entry.value;

    vector<gamemove> moves = node.allmoves();
    gamemove newbestmove;
    int numbestmoves = 0;

    double value = node.isdwarfturn ? INFINITY : -INFINITY;
    for (gamemove move : moves) {
        gamestate newnode = node;
        newnode.domove(move);

        int newdepth = depth-1;
        if (ok_to_extend && depth < 2 && newnode.in_threat(move.to)) {
            ok_to_extend = false;
            newdepth += 1;
        }
        double newvalue = alphabeta(newnode, newdepth, alpha, beta);
        ok_to_extend = true;

        if (top && newvalue == value) {
            numbestmoves += 1;
            uniform_int_distribution<> dist(1,numbestmoves);
            if (dist(randgen) == 1) newbestmove = move;
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

        if (alpha >= beta) break;
    }

    entry.alpha = alpha;
    entry.beta = beta;
    entry.value = value;
    entry.depth = depth;
    ttable_put(entry);

    if (top) bestmove = newbestmove;
    return value;
}

double alphabeta_brain::hotspot_heuristic_evaluation(gamestate node) {
    double score = node.heuristic_score();

    //TODO identify hot spots
    vector<hotspot> hotspots = {};
    for (gamemove & move : node.allmoves()) {
        if (! move.capt) continue;

        hotspots.push_back(hotspot(move));
    }

    //TODO search each hot spot (to current_base_search_depth ?)
    //TODO add each hot spot's minmaxed heuristic score to the base score

    return score;
}

gamemove alphabeta_brain::best_move() {
    return bestmove;
}

void alphabeta_brain::do_move(gamemove move) {
    state.domove(move);
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
