#include "rollout2.hh"

extern "C" {
#include "siphash.c"
}

// create invalid entry
rollout2_entry::rollout2_entry() {
}

// create valid defaults entry
rollout2_entry::rollout2_entry(uint64_t newhash) : hash(newhash) {
    set_checksum();
}

uint64_t zero[2] = {0,0};
uint64_t rollout2_entry::compute_checksum() {
    uint64_t newchecksum;

    siphash((uint8_t *) & hash, (uint8_t *) & checksum - (uint8_t *) & hash,
            (uint8_t *) zero, (uint8_t *) & newchecksum, 8);

    return newchecksum;
}

bool rollout2_entry::valid_checksum() {
    return checksum == compute_checksum();
}

void rollout2_entry::set_checksum() {
    checksum = compute_checksum();
}

rollout2_brain::rollout2_brain(gamestate newstate) : state(newstate) {
    randgen = mt19937(time(NULL));
    ttable_size = TTABLE_SIZE;
    //ttable = new rollout2_entry[ttable_size];
    ttable = (rollout2_entry *) malloc(ttable_size * sizeof(rollout2_entry));
    cout << "ttable allocated" << endl;
}

void rollout2_brain::think_seconds(double seconds) {
    double start_time = get_now();
    double end_time = start_time + seconds;
    while (get_now() < end_time) {
        for (int n=0 ; n<STEPS ; n+=1) {
            rollout2_entry entry_s = ttable_get(state);
            if (entry_s.vminus >= entry_s.vplus) return;
            rollout2(state, entry_s.vminus, entry_s.vplus);
        }
    }
}

double get_now() {
    struct timespec raw_time;
    clock_gettime(CLOCK_REALTIME_COARSE, & raw_time);

    return (double) raw_time.tv_sec + (double) raw_time.tv_nsec * 1e-9;
}

// convert a range -32..32 score to range 0..1 for use in UCT
double to_0_1(double rawscore, bool isdwarfturn) {
    double score = (rawscore + MAX_DWARFS) / (2 * MAX_DWARFS);
    if (isdwarfturn) return 1.0 - score;
    else return score;
}

double rollout2_brain::rollout2(gamestate s, double alpha, double beta) {
    double g;

    rollout2_entry entry_s = ttable_get(s);

    if (s.gameover()) {
        g = s.final_score();
        entry_s.vminus = g;
        entry_s.vplus = g;
    }
    else {
        vector<gamemove> allmoves = s.allmoves();
        map<gamemove,rollout2_entry> entries;

        double maxbeta = -INFINITY;
        double max_uct = -INFINITY;
        int considered = 0;
        gamemove movechoice;
        double alpha_choice;
        double beta_choice;
        for (gamemove move : allmoves) {
            gamestate child = s.child(move);
            rollout2_entry entry_c = ttable_get(child);
            entries[move] = entry_c;

            double alpha_move = max(alpha, entries[move].vminus);
            double beta_move = min(beta, entries[move].vplus);

            if (alpha_move > beta_move) continue;
            if (beta_move < maxbeta) continue;

            // use s.isdwarfturn because we want to know if the prospective
            // move is good for the player at the current state
            double moveuct = to_0_1(entry_c.mu, s.isdwarfturn)
                             + C * sqrt(log(entry_s.visits) / entry_c.visits);
            if (beta_move > maxbeta) {
                maxbeta = beta_move;
                max_uct = moveuct;
                considered = 1;
                movechoice = move;
                alpha_choice = alpha_move;
                beta_choice = beta_move;
            }
            else if (moveuct > max_uct) {
                    max_uct = moveuct;
                    considered = 1;
                    movechoice = move;
                    alpha_choice = alpha_move;
                    beta_choice = beta_move;
            }
            else if (moveuct == max_uct) {
                considered += 1;
                if (random() < 1.0 / considered) {
                    movechoice = move;
                    alpha_choice = alpha_move;
                    beta_choice = beta_move;
                }
            }
        }

        gamestate newstate = s.child(movechoice);
        g = rollout2(newstate, alpha_choice, beta_choice);

        if (s.isdwarfturn) {
            entry_s.vminus = INFINITY; // min of vminus of all s's children
            entry_s.vplus = INFINITY; // min of vplus of all s's children
            for (auto elem : entries) {
                rollout2_entry e = elem.second;
                if (e.vminus < entry_s.vminus) entry_s.vminus = e.vminus;
                if (e.vplus < entry_s.vplus) entry_s.vplus = e.vplus;
            }
        }
        else {
            entry_s.vminus = -INFINITY; // max of vminus of all s's children
            entry_s.vplus = -INFINITY; // max of vplus of all s's children
            for (auto elem : entries) {
                rollout2_entry e = elem.second;
                if (e.vminus > entry_s.vminus) entry_s.vminus = e.vminus;
                if (e.vplus > entry_s.vplus) entry_s.vplus = e.vplus;
            }
        }
    }

    int n_s = entry_s.visits;
    entry_s.mu = entry_s.mu * n_s / (n_s+1.0) + g * 1.0 / (n_s+1.0);
    entry_s.visits += 1;

    ttable_put(entry_s);

    return g;
}

gamemove rollout2_brain::best_move() {
    vector<gamemove> allmoves = state.allmoves();
    for (auto move : allmoves) {
        rollout2_entry entry = ttable_get(state.child(move));
        /*
        cout << move << ": " << entry.mu << ", " << entry.visits
             << ", " << entry.vminus << ".." << entry.vplus << endl;
        */
    }
}

void rollout2_brain::do_move(gamemove move) {
    state.domove(move);
}


rollout2_entry rollout2_brain::ttable_get(gamestate state) {
    rollout2_entry entry = ttable[state.hash % ttable_size];
    if (entry.hash == state.hash && entry.valid_checksum()) return entry;
    else return rollout2_entry(state.hash);
}

void rollout2_brain::ttable_put(rollout2_entry entry) {
    entry.set_checksum();
    ttable[entry.hash % ttable_size] = entry;
}
