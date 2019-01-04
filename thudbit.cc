#include "thudbit.hh"

using namespace std;

void warn(string msg) {
    cerr << msg << endl;
}

coord::coord() : x(-1), y(-1) {
}

coord::coord(int newx, int newy) : x(newx), y(newy) {
}

coord operator*(coord pos, int scalar) {
    return (coord) {pos.x*scalar, pos.y*scalar};
}

coord operator*(int scalar, coord pos) {
    return (coord) {pos.x*scalar, pos.y*scalar};
}

coord operator+(coord pos, coord delta) {
    return (coord) {pos.x+delta.x, pos.y+delta.y};
}

coord operator-(coord pos, coord delta) {
    return (coord) {pos.x-delta.x, pos.y-delta.y};
}

bool operator==(coord left, coord right) {
    return left.x == right.x && left.y == right.y;
}

bool operator!=(coord left, coord right) {
    return left.x != right.x || left.y != right.y;
}

bool operator<(coord left, coord right) {
    if (left.x >= right.x) return false;
    if (left.y >= right.y) return false;
    return true;
}

const string COLS = "ABCDEFGHJKLMNOP";
ostream & operator<<(ostream & out, const coord pos) {
    out << COLS[pos.x] << pos.y+1;
    return out;
}

istream & operator>>(istream & in, coord & pos) {
    char col;
    int row;
    in >> col >> row;

    int x = COLS.find(col);
    int y = row-1;

    pos = {x,y};

    return in;
}

bool coord::inbounds() {
    return 0 <= x && x < SIZE && 0 <= y && y < SIZE;
}

bitref::bitref(uint16_t * newbitline, uint8_t newwhichbit)
        : bitline(newbitline), whichbit(newwhichbit) {
}

bitref::operator bool() const {
    return (bool) (* bitline & (LEFT_COLUMN_BIT >> whichbit));
}

bitref & bitref::operator=(bool newbit) {
    if (newbit) * bitline |= LEFT_COLUMN_BIT >> whichbit;
    else * bitline &= ~(LEFT_COLUMN_BIT >> whichbit);

    return * this;
}

/*
bitref & bitref::operator=(const bitref & newbit) {
    //TODO
}

bool bitref::operator~() const {
    //TODO
}
*/

bitboard::bitboard() : data{} {
}

bitboard::bitboard(bitboard_array newdata) : data(newdata) {
}

bool bitboard::operator[](coord pos) const {
    return (bool) (data[pos.y] & (LEFT_COLUMN_BIT >> pos.x));
}

bitref bitboard::operator[](coord pos) {
    return bitref(& data[pos.y], pos.x);
}

uint8_t neighborbits(bitboard board, coord pos) {
    uint8_t bits = 0;
    for (int n=0 ; n<NUM_DIRS ; n+=1) {
        coord neighbor = pos + dirs[n];
        if (neighbor.inbounds() && board[neighbor]) bits |= 1 << n;
    }

    return bits;
}

gamemove::gamemove() {
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto) {
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto,
                   bool newcapt)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto),
          capt(newcapt) {
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto,
                   bool newcapt, uint8_t newcapts)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto),
          capt(newcapt), capts(newcapts) {
}

bool operator==(gamemove left, gamemove right) {
    if (left.isdwarfmove != right.isdwarfmove) return false;
    if (left.from != right.from) return false;
    if (left.to != right.to) return false;
    if (left.capt != right.capt) return false;
    if (left.capts != right.capts) return false;

    return true;
}

bool operator!=(gamemove left, gamemove right) {
    return ! (left == right);
}

bool operator<(gamemove left, gamemove right) {
    if (left.isdwarfmove >= right.isdwarfmove) return false;
    if (! (left.from < right.from)) return false;
    if (! (left.to < right.to)) return false;
    if (left.capt >= right.capt) return false;
    if (left.capts >= right.capts) return false;

    return true;
}

ostream & operator<<(ostream & out, const gamemove move) {
    out << (move.isdwarfmove ? 'd' : 'T') << ' ';
    out << move.from << '-' << move.to;
    if (move.capt) {
        if (move.capts == 0) out << 'x' << move.to;
        else for (int n=0 ; n<NUM_DIRS ; n+=1) {
            if (move.capts & (uint8_t(1) << n)) {
                coord attack = move.to + dirs[n];
                out << 'x' << attack;
            }
        }
    }

    return out;
}

istream & operator>>(istream & in, gamemove & move) {
    char whichside;
    in >> ws >> whichside >> ws;
    if (whichside == 'd') move.isdwarfmove = true;
    else if (whichside == 'T') move.isdwarfmove = false;
    else warn("bad move");

    char dash;
    in >> move.from >> ws >> dash >> ws >> move.to >> ws;
    if (dash != '-') warn("bad move");

    while (in.peek() == 'x') {
        in.get();
        coord attack;
        in >> ws >> attack >> ws;
    }

    return in;
}

void gamemove::show() {
    cerr << * this << endl;
}

zobrist_hashes gamestate::gen_hashes() {
    mt19937_64 randgen = mt19937_64(0xfedcba9876543210ULL);
    uniform_int_distribution<uint64_t> dist(0,numeric_limits<uint64_t>::max());

    zobrist_hashes hs;
    hs.turn = dist(randgen);
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            hs.dwarfs[y][x] = dist(randgen);
            hs.trolls[y][x] = dist(randgen);
        }
    }

    return hs;
}

zobrist_hashes gamestate::hashes = gen_hashes();

gamestate::gamestate() : isdwarfturn(true), sincecapt(0),
                         numdwarfs(0), numtrolls(0), board(), hash(0) {
    hash = 0;

    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            coord pos(x,y);
            if (defaultdwarfs[pos]) {
                board.dwarfs[pos] = true;
                hash ^= hashes.dwarfs[pos.y][pos.x];
                numdwarfs += 1;
            }
            else if (defaulttrolls[pos]) {
                board.trolls[pos] = true;
                hash ^= hashes.trolls[pos.y][pos.x];
                numtrolls += 1;
            }
        }
    }
}

/*
void gamestate::calculate_dwarfmobility() {
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        dwarfmobility[ix] = {};
        if (! dwarfs[ix].alive) continue;
        coord pos = dwarfs[ix].pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            int mobility = 0;
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos + dist*delta;
                if (! check.inbounds()) break;
                else if (blocks[check]
                         || dwarfmap[check]
                         || trollmap[check]) break;
                else mobility = dist;
            }
            dwarfmobility[ix][n] = mobility;
        }
    }
}
*/

/*
void gamestate::update_dwarfmobility(coord where) {
    boardmap occupied = blocks || dwarfmap || trollmap;

    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) {
            dwarfmobility[ix] = {};
            continue;
        }
        coord pos = dwarf.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            int mobility = 0;
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos + dist*delta;
                if (! check.inbounds()) break;
                else if (occupied[check.y][check.x]) break;
                else mobility = dist;
            }
            dwarfmobility[ix][n] = mobility;
        }
    }
}
*/

/*
void gamestate::calculate_dwarfthreats() {
    dwarfthreats = {};

    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        if (! dwarfs[ix].alive) continue;
        coord pos = dwarfs[ix].pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos - (dist-1)*delta;
                coord attack = pos + dist*delta;
                if (! check.inbounds()) break;
                if (! attack.inbounds()) break;
                if (! dwarfmap[check]) break;
                if (trollmap[attack]) {
                    dwarfthreats[attack] = true;
                    break;
                }
                if (dwarfmap[attack]) break;
                if (blocks[attack]) break;
                dwarfthreats[attack] = true;
            }
        }
    }
}
*/

/*
void gamestate::calculate_trollthreats() {
    trollthreats = {};

    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        if (! trolls[ix].alive) continue;
        coord pos = trolls[ix].pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos - (dist-1)*delta;
                coord shoveto = pos + dist*delta;
                if (! check.inbounds()) break;
                if (! shoveto.inbounds()) break;
                if (! trollmap[check]) break;
                if (blocks[shoveto]
                    || dwarfmap[shoveto]
                    || trollmap[shoveto]) break;
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = shoveto + dirs[nn];
                    if (! attack.inbounds()) continue;
                    if (blocks[attack]) continue;
                    trollthreats[attack] = true;
                }
            }
        }
    }
}
*/

int popcount(uint16_t bits) {
    return __builtin_popcount(bits);
}

int leading0s(uint16_t bits) {
    return __builtin_clzll(bits) - 48;
}

bool gamestate::valid() {
    int dwarfcount = 0;
    int trollcount = 0;
    uint64_t checkhash = 0;
    for (int y=0 ; y<SIZE ; y+=1) {
        // check for intersections between dwarfs, trolls, and blocks
        if (board.blocks.data[y] & board.dwarfs.data[y]
            || board.blocks.data[y] & board.trolls.data[y]
            || board.dwarfs.data[y] & board.trolls.data[y]) {
            warn("occupied");
            return false;
        }

        // check for dwarfs and trolls out of bounds
        if (board.dwarfs.data[y] & 0b1000000000000000
            || board.trolls.data[y] & 0b1000000000000000) {
            warn("bounds");
            return false;
        }

        // check dwarf and troll counts
        dwarfcount += popcount(board.dwarfs.data[y]);
        trollcount += popcount(board.trolls.data[y]);

        // update piece position hashes
        uint16_t dwarfline = board.dwarfs.data[y];
        while (dwarfline) {
            int x = leading0s(dwarfline) - 1;
            dwarfline &= ~ (LEFT_COLUMN_BIT >> x);

            checkhash ^= hashes.dwarfs[y][x];
        }
        uint16_t trollline = board.trolls.data[y];
        while (trollline) {
            int x = leading0s(trollline) - 1;
            trollline &= ~ (LEFT_COLUMN_BIT >> x);

            checkhash ^= hashes.trolls[y][x];
        }
    }
    if (dwarfcount != numdwarfs) {warn("dwarfcount"); return false;}
    if (trollcount != numtrolls) {warn("trollcount"); return false;}

    // check hash value
    if (! isdwarfturn) checkhash ^= hashes.turn;
    if (checkhash != hash) {warn("hash"); return false;}

    // no problems found
    return true;
}

vector<gamemove> gamestate::allmoves() {
    if (gameover()) return vector<gamemove>();
    else if (isdwarfturn) return alldwarfmoves();
    else return alltrollmoves();
}

vector<gamemove> gamestate::alldwarfmoves() {
    vector<gamemove> allmoves = {};

    for (int y=0 ; y<SIZE ; y+=1) {
        uint16_t dwarfline = board.dwarfs.data[y];
        while (dwarfline) {
            int x = leading0s(dwarfline) - 1;
            dwarfline &= ~ (LEFT_COLUMN_BIT >> x);

            coord from = coord(x,y);
            for (int n=0 ; n<NUM_DIRS ; n+=1) {
                for (int dist=1 ; dist<=SIZE ; dist+=1) {
                    coord check = from - (dist-1)*dirs[n];
                    if (! check.inbounds()) break;
                    if (! board.dwarfs[check]) break;
                    coord to = from + dist*dirs[n];
                    if (! to.inbounds()) break;
                    if (board.blocks[to]) break;
                    if (board.dwarfs[to]) break;
                    if (board.trolls[to]) {
                        gamemove move = gamemove(true, from, to, true);
                        allmoves.push_back(move);
                        break;
                    }
                }

                for (int dist=1 ; dist<=SIZE ; dist+=1) {
                    coord to = from + dist*dirs[n];
                    if (! to.inbounds()) break;
                    if (board.blocks[to]) break;
                    if (board.dwarfs[to]) break;
                    if (board.trolls[to]) break;
                    allmoves.push_back(gamemove(true, from, to));
                }
            }
        }
    }

    return allmoves;
}

vector<gamemove> gamestate::alltrollmoves() {
    vector<gamemove> allmoves = {};

    for (int y=0 ; y<SIZE ; y+=1) {
        uint16_t trollline = board.trolls.data[y];
        while (trollline) {
            int x = leading0s(trollline) - 1;
            trollline &= ~ (LEFT_COLUMN_BIT >> x);

            coord from = coord(x,y);
            for (int n=0 ; n<NUM_DIRS ; n+=1) {
                for (int dist=1 ; dist<=SIZE ; dist+=1) {
                    coord check = from - (dist-1)*dirs[n];
                    // can't shove unless at least 2 trolls
                    if (dist == 1) check = from - dist*dirs[n];
                    if (! check.inbounds()) break;
                    if (! board.trolls[check]) break;
                    coord to = from + dist*dirs[n];
                    if (! to.inbounds()) break;
                    if (board.blocks[to]) break;
                    if (board.trolls[to]) break;
                    if (board.dwarfs[to]) break;

                    uint8_t capts = neighborbits(board.dwarfs, to);
                    if (capts) {
                        gamemove move = gamemove(false, from, to, true, capts);
                        allmoves.push_back(move);
                    }
                }

                coord to = from + dirs[n];
                if (! to.inbounds()) break;
                if (! board.blocks[to]
                 && ! board.trolls[to]
                 && ! board.dwarfs[to]) {
                    allmoves.push_back(gamemove(false, from, to));
                    for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                        coord attack = to + dirs[nn];
                        if (attack.inbounds() && board.dwarfs[attack]) {
                            uint8_t capts = (uint8_t) 1 << nn;
                            gamemove move = gamemove(false, from, to, true, capts);
                            allmoves.push_back(move);
                        }
                    }
                }
            }
        }
    }

    return allmoves;
}

bool gamestate::validmove(gamemove move) {
    vector<gamemove> moves = allmoves();
    for (gamemove checkmove : moves) {
        if (move == checkmove) return true;
    }
    return false;
}

void gamestate::domove(gamemove move) {
    if (move.isdwarfmove) dodwarfmove(move);
    else dotrollmove(move);
}

void gamestate::dodwarfmove(gamemove move) {
    board.dwarfs[move.from] = false;
    hash ^= hashes.dwarfs[move.from.y][move.from.x];
    board.dwarfs[move.to] = true;
    hash ^= hashes.dwarfs[move.to.y][move.to.x];

    if (move.capt) {
        sincecapt = 0;
        board.trolls[move.to] = false;
        hash ^= hashes.trolls[move.to.y][move.to.x];
        numtrolls -= 1;
    }
    else sincecapt += 1;

    isdwarfturn = false;
    hash ^= hashes.turn;
}

void gamestate::dotrollmove(gamemove move) {
    board.trolls[move.from] = false;
    hash ^= hashes.trolls[move.from.y][move.from.x];
    board.trolls[move.to] = true;
    hash ^= hashes.trolls[move.to.y][move.to.x];

    if (move.capt) {
        sincecapt = 0;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            if (move.capts & (uint8_t(1) << n)) {
                coord attack = move.to + dirs[n];
                board.dwarfs[attack] = false;
                hash ^= hashes.dwarfs[attack.y][attack.x];
                numdwarfs -= 1;
            }
        }
    }
    else sincecapt += 1;

    isdwarfturn = true;
    hash ^= hashes.turn;
}

gamestate gamestate::child(gamemove move) {
    gamestate newstate = * this;
    newstate.domove(move);
    return newstate;
}

bool gamestate::gameover() {
    bool allcapt = numdwarfs == 0 || numtrolls == 0;
    bool captmade = numdwarfs<MAX_DWARFS || numtrolls<MAX_TROLLS;
    return allcapt || sincecapt > 20 || captmade && sincecapt > 10;
}

double gamestate::final_score() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}

double gamestate::heuristic_score() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}
