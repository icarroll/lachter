#include "lachter.hh"

using namespace std;

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

const char COLS[] = "ABCDEFGHJKLMNOP";
ostream & operator<<(ostream & out, const coord pos) {
    out << COLS[pos.x] << pos.y+1;
    return out;
}

bool coord::inbounds() {
    return 0 <= x && x < SIZE && 0 <= y && y < SIZE;
}

piecestate::piecestate() : alive(false), pos(NOWHERE) {
}

piecestate::piecestate(coord newpos) : alive(true), pos(newpos) {
}

boardmap::boardmap() : data() {
    fill(& data[0][0], & data[SIZE][0], NOBODY);
}

bool boardmap::operator[](coord pos) const {
    return data[pos.y][pos.x] != NOBODY;
}

int & boardmap::which(coord pos) {
    return data[pos.y][pos.x];
}

bool boardmap::operator!=(boardmap & that) {
    return data != that.data;
}

boardflags::boardflags() : data() {
}

boardflags::boardflags(boardflags_array newdata) : data(newdata) {
}

bool & boardflags::operator[](coord pos) {
    return data[pos.y][pos.x];
}

bool boardflags::operator[](coord pos) const {
    return data[pos.y][pos.x];
}

bool boardflags::operator!=(boardflags & that) {
    return data != that.data;
}

uint8_t neighborbits(boardmap map, coord pos) {
    uint8_t bits = 0;
    for (int n=0 ; n<NUM_DIRS ; n+=1) {
        coord neighbor = pos + dirs[n];
        if (neighbor.inbounds() && map[neighbor]) bits |= 1 << n;
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

gamestate::gamestate() : isdwarfturn(true), sincecapt(0),
                         numdwarfs(0), dwarfmap(),
                         numtrolls(0), trollmap() {
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            coord pos(x,y);
            if (dwarfstart[pos]) {
                dwarfs[numdwarfs] = piecestate(pos);
                dwarfmap.which(pos) = numdwarfs;
                numdwarfs += 1;
            }
            else if (trollstart[pos]) {
                trolls[numtrolls] = piecestate(pos);
                trollmap.which(pos) = numtrolls;
                numtrolls += 1;
            }
        }
    }

    calculate_dwarfmobility();
    calculate_dwarfthreats();
    calculate_trollthreats();
}

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

void warn(string msg) {
    cerr << msg << endl;
}

bool gamestate::valid() {
    boardflags occupied = blocks;
    boardmap checkdwarfmap = {};
    boardmap checktrollmap = {};

    // all pieces at unique legal coords
    int dwarfcount = 0;
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (! pos.inbounds()) {warn("bounds"); return false;}
        if (occupied[pos]) {warn("occupied"); return false;}
        occupied[pos] = true;
        checkdwarfmap.which(pos) = ix;
        dwarfcount += 1;
        //cout << "counted a dwarf" << endl;
    }
    if (checkdwarfmap != dwarfmap)
    {
        warn("dwarfmap");
        return false;
    }
    /*
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            if (checkdwarfmap[y][x] != dwarfmap[y][x])
            {
                warn("dwarfmap");
                return false;
            }
        }
    }
    */

    int trollcount = 0;
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (! pos.inbounds()) {warn("bounds"); return false;}
        if (occupied[pos]) {warn("occupied"); return false;}
        occupied[pos] = true;
        checktrollmap.which(pos) = ix;
        trollcount += 1;
        //cout << "counted a troll" << endl;
    }
    if (checktrollmap != trollmap)
    {
        warn("trollmap");
        return false;
    }
    /*
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            if (checktrollmap[y][x] != trollmap[y][x])
            {
                warn("trollmap");
                return false;
            }
        }
    }
    */

    // piece counts accurate
    if (dwarfcount != numdwarfs) {warn("dwarfcount"); return false;}
    if (trollcount != numtrolls) {warn("trollcount"); return false;}

    // dwarf mobility accurate
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) {
            if (dwarfmobility[ix] != array<int,NUM_DIRS>()) {
                warn("mobility");
                return false;
            }
            continue;
        }
        coord pos = dwarf.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            int mobility = 0;
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos + dist*delta;
                if (! check.inbounds()) break;
                else if (occupied[check]) break;
                else mobility = dist;
            }
            if (dwarfmobility[ix][n] != mobility) {
                warn("mobility");
                return false;
            }
        }
    }

    // threat maps accurate
    boardflags checkdwarfthreats = {};
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos - (dist-1)*delta;
                coord attack = pos + dist*delta;
                if (! check.inbounds()) break;
                if (! attack.inbounds()) break;
                if (! dwarfmap[check]) break;
                if (occupied[attack]) {
                    if (trollmap[attack]) {
                        checkdwarfthreats[attack] = true;
                    }
                    break;
                }
                checkdwarfthreats[attack] = true;
            }
        }
    }
    if (checkdwarfthreats != dwarfthreats) {warn("dwarfthreats"); return false;}

    boardflags checktrollthreats = {};
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            coord delta = dirs[n];
            for (int dist=1 ; dist<SIZE ; dist+=1) {
                coord check = pos - (dist-1)*delta;
                coord shoveto = pos + dist*delta;
                if (! check.inbounds()) break;
                if (! shoveto.inbounds()) break;
                if (! trollmap[check]) break;
                if (occupied[shoveto]) break;
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = shoveto + dirs[nn];
                    if (! attack.inbounds()) continue;
                    if (blocks[attack]) continue;
                    checktrollthreats[attack] = true;
                }
            }
        }
    }
    if (checktrollthreats != trollthreats) {warn("trollthreats"); return false;}

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

    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        if (! dwarfs[ix].alive) continue;
        coord from = dwarfs[ix].pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            for (int dist=1 ; dist<=SIZE ; dist+=1) {
                coord check = from - (dist-1)*dirs[n];
                if (! check.inbounds()) break;
                if (! dwarfmap[check]) break;
                coord to = from + dist*dirs[n];
                if (! to.inbounds()) break;
                if (blocks[to]) break;
                if (dwarfmap[to]) break;
                if (trollmap[to]) {
                    gamemove move = gamemove(true, from, to, true);
                    allmoves.push_back(move);
                    break;
                }
            }

            for (int dist=1 ; dist<=dwarfmobility[ix][n] ; dist+=1) {
                coord to = from + dist*dirs[n];
                if (! to.inbounds()) warn("bad mobility");
                allmoves.push_back(gamemove(true, from, to));
            }
        }
    }

    return allmoves;
}

vector<gamemove> gamestate::alltrollmoves() {
    vector<gamemove> allmoves = {};

    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord from = troll.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            for (int dist=1 ; dist<=SIZE ; dist+=1) {
                coord check = from - (dist-1)*dirs[n];
                // can't shove unless at least 2 trolls
                if (dist == 1) check = from - dist*dirs[n];
                if (! check.inbounds()) break;
                if (! trollmap[check]) break;
                coord to = from + dist*dirs[n];
                if (! to.inbounds()) break;
                if (blocks[to]) break;
                if (trollmap[to]) break;
                if (dwarfmap[to]) break;

                uint8_t capts = neighborbits(dwarfmap, to);
                if (capts) {
                    gamemove move = gamemove(false, from, to, true, capts);
                    allmoves.push_back(move);
                }
            }

            coord to = from + dirs[n];
            if (! to.inbounds()) break;
            if (! blocks[to]
             && ! trollmap[to]
             && ! dwarfmap[to]) {
                allmoves.push_back(gamemove(false, from, to));
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = to + dirs[nn];
                    if (attack.inbounds() && dwarfmap[attack]) {
                        uint8_t capts = (uint8_t) 1 << nn;
                        gamemove move = gamemove(false, from, to, true, capts);
                        allmoves.push_back(move);
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
    int dwarfix = dwarfmap.which(move.from);
    dwarfs[dwarfix].pos = move.to;
    dwarfmap.which(move.from) = NOBODY;
    dwarfmap.which(move.to) = dwarfix;

    if (move.capt) {
        sincecapt = 0;
        int trollix = trollmap.which(move.to);
        trolls[trollix].alive = false;
        trolls[trollix].pos = NOWHERE;
        trollmap.which(move.to) = NOBODY;
        numtrolls -= 1;
    }
    else sincecapt += 1;

    isdwarfturn = false;

    calculate_dwarfmobility();
    calculate_dwarfthreats();
    calculate_trollthreats();
}

void gamestate::dotrollmove(gamemove move) {
    int trollix = trollmap.which(move.from);
    trolls[trollix].pos = move.to;
    trollmap.which(move.from) = NOBODY;
    trollmap.which(move.to) = trollix;

    if (move.capt) {
        sincecapt = 0;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            if (move.capts & (uint8_t(1) << n)) {
                coord attack = move.to + dirs[n];
                int dwarfix = dwarfmap.which(attack);
                dwarfs[dwarfix].alive = false;
                dwarfs[dwarfix].pos = NOWHERE;
                dwarfmap.which(attack) = NOBODY;
                numdwarfs -= 1;
            }
        }
    }
    else sincecapt += 1;

    isdwarfturn = true;

    calculate_dwarfmobility();
    calculate_dwarfthreats();
    calculate_trollthreats();
}

bool gamestate::gameover() {
    bool allcapt = numdwarfs == 0 || numtrolls == 0;
    bool captmade = numdwarfs<MAX_DWARFS || numtrolls<MAX_TROLLS;
    return allcapt || sincecapt > 20 || captmade && sincecapt > 10;
}

float gamestate::final_score() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}

float gamestate::heuristic_score() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}
