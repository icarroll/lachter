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

bool coord::inbounds() {
    return 0 <= x && x < SIZE && 0 <= y && y < SIZE;
}

piecestate::piecestate() : alive(false), pos(-1,-1), inthreat(false) {
}

piecestate::piecestate(bool newalive, coord newpos, bool newinthreat)
        : alive(newalive), pos(newpos), inthreat(newinthreat) {
}

uint8_t neighborbits(boardmap map, coord pos) {
    uint8_t bits = 0;
    for (int n=0 ; n<NUM_DIRS ; n+=1) {
        coord neighbor = pos + dirs[n];
        if (map[neighbor.y][neighbor.x]) bits |= 1 << n;
    }

    return bits;
}

boardmap operator||(boardmap left, boardmap right) {
    boardmap result = {};
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            result[y][x] = left[y][x] || right[y][x];
        }
    }

    return result;
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto),
          capt(false), capts(0) {
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto,
                   bool newcapt)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto),
          capt(newcapt), capts(0) {
}

gamemove::gamemove(bool newisdwarfmove, coord newfrom, coord newto,
                   bool newcapt, uint8_t newcapts)
        : isdwarfmove(newisdwarfmove), from(newfrom), to(newto),
          capt(false), capts(newcapts) {
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

gamestate::gamestate() : isdwarfturn(true), sincecapt(0),
                         numdwarfs(0), numtrolls(0) {
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            if (dwarfstart[y][x]) {
                //cout << "found a dwarf start" << endl;
                dwarfs[numdwarfs] = piecestate(true, coord(x,y), false);
                numdwarfs += 1;
            }
            else if (trollstart[y][x]) {
                //cout << "found a troll start" << endl;
                trolls[numtrolls] = piecestate(true, coord(x,y), false);
                numtrolls += 1;
            }
        }
    }

    /*
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        cout << "checking dwarf " << ix << " is " << (dwarfs[ix].alive ? "alive" : "dead") << " at " << dwarfs[ix].pos.x << "," << dwarfs[ix].pos.y << endl;
    }
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        cout << "checking troll " << ix << " is " << (trolls[ix].alive ? "alive" : "dead") << " at " << trolls[ix].pos.x << "," << trolls[ix].pos.y << endl;
    }
    */

    calculate_maps();
    calculate_dwarfmobility();
    calculate_dwarfthreats();
    calculate_trollthreats();
}

void gamestate::calculate_maps() {
    dwarfmap = {};
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        if (! dwarfs[ix].alive) continue;
        coord pos = dwarfs[ix].pos;
        dwarfmap[pos.y][pos.x] = true;
    }

    trollmap = {};
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        trollmap[pos.y][pos.x] = true;
    }
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
                else if (blocks[check.y][check.x]
                         || dwarfmap[check.y][check.x]
                         || trollmap[check.y][check.x]) break;
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
                if (! dwarfmap[check.y][check.x]) break;
                if (trollmap[attack.y][attack.x]) {
                    dwarfthreats[attack.y][attack.x] = true;
                    break;
                }
                if (dwarfmap[attack.y][attack.x]) break;
                if (blocks[attack.y][attack.x]) break;
                dwarfthreats[attack.y][attack.x] = true;
            }
        }
    }

    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        if (! trolls[ix].alive) continue;
        trolls[ix].inthreat = dwarfthreats[trolls[ix].pos.y][trolls[ix].pos.x];
    }
}

void gamestate::calculate_trollthreats() {
    trollthreats = {};

    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
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
                if (! trollmap[check.y][check.x]) break;
                if (blocks[shoveto.y][shoveto.x]
                    || dwarfmap[shoveto.y][shoveto.x]
                    || trollmap[shoveto.y][shoveto.x]) break;
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = shoveto + dirs[nn];
                    if (! attack.inbounds()) continue;
                    if (blocks[attack.y][attack.x]) continue;
                    trollthreats[attack.y][attack.x] = true;
                }
            }
        }
    }

    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        if (! dwarfs[ix].alive) continue;
        dwarfs[ix].inthreat = trollthreats[dwarfs[ix].pos.y][dwarfs[ix].pos.x];
    }
}

void warn(string msg) {
    cerr << msg << endl;
}

bool gamestate::valid() {
    boardmap occupied = blocks;
    boardmap checkdwarfmap = {};
    boardmap checktrollmap = {};

    // all pieces at unique legal coords
    int dwarfcount = 0;
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (! pos.inbounds()) {warn("bounds"); return false;}
        if (occupied[pos.y][pos.x]) {warn("occupied"); return false;}
        occupied[pos.y][pos.x] = true;
        checkdwarfmap[pos.y][pos.x] = true;
        dwarfcount += 1;
        //cout << "counted a dwarf" << endl;
    }
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            if (checkdwarfmap[y][x] != dwarfmap[y][x])
            {
                warn("dwarfmap");
                return false;
            }
        }
    }

    int trollcount = 0;
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (! pos.inbounds()) {warn("bounds"); return false;}
        if (occupied[pos.y][pos.x]) {warn("occupied"); return false;}
        occupied[pos.y][pos.x] = true;
        checktrollmap[pos.y][pos.x] = true;
        trollcount += 1;
        //cout << "counted a troll" << endl;
    }
    for (int y=0 ; y<SIZE ; y+=1) {
        for (int x=0 ; x<SIZE ; x+=1) {
            if (checktrollmap[y][x] != trollmap[y][x])
            {
                warn("trollmap");
                return false;
            }
        }
    }

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
                else if (occupied[check.y][check.x]) break;
                else mobility = dist;
            }
            if (dwarfmobility[ix][n] != mobility) {
                warn("mobility");
                return false;
            }
        }
    }

    // threat maps accurate
    boardmap checkdwarfthreats = {};
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
                if (! dwarfmap[check.y][check.x]) break;
                if (occupied[attack.y][attack.x]) {
                    if (trollmap[attack.y][attack.x]) {
                        checkdwarfthreats[attack.y][attack.x] = true;
                    }
                    break;
                }
                checkdwarfthreats[attack.y][attack.x] = true;
            }
        }
    }
    if (checkdwarfthreats != dwarfthreats) {warn("dwarfthreats"); return false;}

    boardmap checktrollthreats = {};
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
                if (! trollmap[check.y][check.x]) break;
                if (occupied[shoveto.y][shoveto.x]) break;
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = shoveto + dirs[nn];
                    if (! attack.inbounds()) continue;
                    if (blocks[attack.y][attack.x]) continue;
                    checktrollthreats[attack.y][attack.x] = true;
                }
            }
        }
    }
    if (checktrollthreats != trollthreats) {warn("trollthreats"); return false;}

    // piece inthreat accurate
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (dwarf.inthreat != trollthreats[pos.y][pos.x]) {
            warn("dwarfthreat");
            return false;
        }
    }
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (troll.inthreat != dwarfthreats[pos.y][pos.x]) {
            warn("dwarfthreat");
            return false;
        }
    }

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
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord from = dwarf.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            for (int dist=1 ; dist<=SIZE ; dist+=1) {
                coord check = from - (dist-1)*dirs[n];
                if (! check.inbounds()) break;
                if (! dwarfmap[check.y][check.x]) break;
                coord to = from + dist*dirs[n];
                if (! to.inbounds()) break;
                if (blocks[to.y][to.x]) break;
                if (dwarfmap[to.y][to.x]) break;
                if (trollmap[to.y][to.x]) {
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
                if (! trollmap[check.y][check.x]) break;
                coord to = from + dist*dirs[n];
                if (! to.inbounds()) break;
                if (blocks[to.y][to.x]) break;
                if (trollmap[to.y][to.x]) break;
                if (dwarfmap[to.y][to.x]) break;

                uint8_t capts = neighborbits(dwarfmap, to);
                if (capts) {
                    gamemove move = gamemove(false, from, to, true, capts);
                    allmoves.push_back(move);
                }
            }

            coord to = from + dirs[n];
            if (! to.inbounds()) break;
            if (! blocks[to.y][to.x]
             && ! trollmap[to.y][to.x]
             && ! dwarfmap[to.y][to.x]) {
                allmoves.push_back(gamemove(false, from, to));
                for (int nn=0 ; nn<NUM_DIRS ; nn+=1) {
                    coord attack = to + dirs[nn];
                    if (dwarfmap[attack.y][attack.x]) {
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
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        if (! dwarfs[ix].alive) continue;
        if (dwarfs[ix].pos == move.from) {
            isdwarfturn = false;
            dwarfs[ix].pos = move.to;
            dwarfmap[move.from.y][move.from.x] = false;
            dwarfmap[move.to.y][move.to.x] = true;
            if (move.capt) {
                sincecapt = 0;
                for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
                    if (! trolls[ix].alive) continue;
                    if (trolls[ix].pos == move.to) {
                        trolls[ix].alive = false;
                        trolls[ix].pos = coord(-1,-1);
                        trollmap[move.to.y][move.to.x] = false;
                        numtrolls -= 1;
                        break;
                    }
                }
            }
            else sincecapt += 1;

            calculate_dwarfmobility();
            calculate_dwarfthreats();
            calculate_trollthreats();

            break;
        }
    }
}

void gamestate::dotrollmove(gamemove move) {
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        if (! trolls[ix].alive) continue;
        if (trolls[ix].pos == move.from) {
            isdwarfturn = true;
            trolls[ix].pos = move.to;
            trollmap[move.from.y][move.from.x] = false;
            trollmap[move.to.y][move.to.x] = true;
            if (move.capt) {
                sincecapt = 0;
                for (int n=0 ; n<NUM_DIRS ; n+=1) {
                    if (move.capts & (uint8_t(1) << n)) {
                        coord attack = move.to + dirs[n];
                        for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
                            if (! dwarfs[ix].alive) continue;
                            if (dwarfs[ix].pos == attack) {
                                dwarfs[ix].alive = false;
                                dwarfs[ix].pos = coord(-1,-1);
                                dwarfmap[attack.y][attack.x] = false;
                                numdwarfs -= 1;
                            }
                        }
                    }
                }
            }
            else sincecapt += 1;

            calculate_dwarfmobility();
            calculate_dwarfthreats();
            calculate_trollthreats();

            break;
        }
    }
}

bool gamestate::gameover() {
    bool allcapt = numdwarfs == 0 || numtrolls == 0;
    bool captmade = numdwarfs<MAX_DWARFS || numtrolls<MAX_TROLLS;
    return allcapt || sincecapt > 20 || captmade && sincecapt > 10;
}

float gamestate::final_score() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}

float gamestate::heuristic() {
    return numtrolls * 4.0 - numdwarfs * 1.0;
}
