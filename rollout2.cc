#include "rollout2.hh"

extern "C" {
#include "siphash.c"
}

// create invalid entry
rollout2_entry::rollout2_entry() {
}

// create valid defaults entry
rollout2_entry::rollout2_entry(uint64_t newhash) : hash(newhash) {
    checksum = compute_checksum();
}

uint64_t zero[2] = {0,0};
uint64_t rollout2_entry::compute_checksum() {
    uint64_t newchecksum;

    siphash((uint8_t *) & hash, (uint8_t *) & checksum - (uint8_t *) & hash,
            (uint8_t *) zero, (uint8_t *) & newchecksum, 8);

    return newchecksum;
}

rollout2_brain::rollout2_brain(gamestate newstate) : state(newstate) {
    randgen = mt19937(time(NULL));
    ttable.reserve(TTABLE_SIZE);
}

void rollout2_brain::think_seconds(double seconds) {
}

// convert a range -32..32 score to range 0..1 for use in UCT
double to_0_1(double score) {
    return (score + MAX_DWARFS) / (2 * MAX_DWARFS);
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

            double moveuct = to_0_1(entry_c.mu)
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
    entry_s.mu = entry_s.mu * n_s / (n_s+1) + g * 1 / (n_s+1);
    entry_s.visits += 1;

    ttable_put(entry_s);

    return g;
}

gamemove rollout2_brain::best_move() {
}

void rollout2_brain::do_move(gamemove move) {
    state.domove(move);
}


rollout2_entry rollout2_brain::ttable_get(gamestate state) {
}

void rollout2_brain::ttable_put(rollout2_entry entry) {
}

