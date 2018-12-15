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

gamestate::gamestate() : isdwarfturn(true), numdwarfs(0), numtrolls(0) {
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
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
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
    boardmap occupied = blocks || dwarfmap || trollmap;
    dwarfthreats = {};

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
                        dwarfthreats[attack.y][attack.x] = true;
                    }
                    break;
                }
                dwarfthreats[attack.y][attack.x] = true;
            }
        }
    }
}

void gamestate::calculate_trollthreats() {
    boardmap occupied = blocks || dwarfmap || trollmap;
    trollthreats = {};

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
                    trollthreats[attack.y][attack.x] = true;
                }
            }
        }
    }
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
        if (! pos.inbounds()) {cerr << "bounds" << endl; return false;}
        if (occupied[pos.y][pos.x]) {cerr << "occupied" << endl; return false;}
        occupied[pos.y][pos.x] = true;
        checkdwarfmap[pos.y][pos.x] = true;
        dwarfcount += 1;
        //cout << "counted a dwarf" << endl;
    }
    if (checkdwarfmap != dwarfmap) {cerr << "dwarfmap" << endl; return false;}

    int trollcount = 0;
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (! pos.inbounds()) {cerr << "bounds" << endl; return false;}
        if (occupied[pos.y][pos.x]) {cerr << "occupied" << endl; return false;}
        occupied[pos.y][pos.x] = true;
        checktrollmap[pos.y][pos.x] = true;
        trollcount += 1;
        //cout << "counted a troll" << endl;
    }
    if (checktrollmap != trollmap) {cerr << "trollmap" << endl; return false;}

    // piece counts accurate
    if (dwarfcount != numdwarfs) {cerr << "dwarfcount" << endl; return false;}
    if (trollcount != numtrolls) {cerr << "trollcount" << endl; return false;}

    // dwarf mobility accurate
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) {
            if (dwarfmobility[ix] != array<int,NUM_DIRS>()) return false;
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
            if (dwarfmobility[ix][n] != mobility) return false;
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
    if (checkdwarfthreats != dwarfthreats) return false;

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
    if (checktrollthreats != trollthreats) return false;

    // piece inthreat accurate
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (dwarf.inthreat != trollthreats[pos.y][pos.x]) return false;
    }
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (troll.inthreat != dwarfthreats[pos.y][pos.x]) return false;
    }

    // no problems found
    return true;
}

vector<gamemove> gamestate::allmoves() {
    if (isdwarfturn) return alldwarfmoves();
    else return alltrollmoves();
}

vector<gamemove> gamestate::alldwarfmoves() {
    vector<gamemove> allmoves;

    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord from = dwarf.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            for (int dist=1 ; dist<=SIZE ; dist+=1) {
                coord check = from - (dist-1)*dirs[n];
                coord to = from + dist*dirs[n];
                if (! dwarfmap[check.y][check.x]) break;
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
                allmoves.push_back(gamemove(true, from, to));
            }
        }
    }

    return allmoves;
}

vector<gamemove> gamestate::alltrollmoves() {
    vector<gamemove> allmoves;

    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord from = troll.pos;
        for (int n=0 ; n<NUM_DIRS ; n+=1) {
            for (int dist=1 ; dist<=SIZE ; dist+=1) {
                coord check = from - (dist-1)*dirs[n];
                // can't shove unless at least 2 trolls
                if (dist == 1) check = from - dist*dirs[n];
                if (! trollmap[check.y][check.x]) break;
                coord to = from + dist*dirs[n];
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
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        if (dwarf.pos == move.from) {
            isdwarfturn = false;
            dwarf.pos = move.to;
            dwarfmap[move.from.y][move.from.x] = false;
            dwarfmap[move.to.y][move.to.x] = true;
            if (move.capt) {
                for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
                    piecestate troll = trolls[ix];
                    if (! troll.alive) continue;
                    if (troll.pos == move.to) {
                        troll.alive = false;
                        troll.pos = coord(-1,-1);
                        trollmap[move.to.y][move.to.x] = false;
                        numtrolls -= 1;
                        break;
                    }
                }
            }

            calculate_dwarfmobility();
            calculate_dwarfthreats();
            calculate_trollthreats();

            break;
        }
    }
}

void gamestate::dotrollmove(gamemove move) {
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        if (troll.pos == move.from) {
            isdwarfturn = true;
            troll.pos = move.to;
            trollmap[move.from.y][move.from.x] = false;
            trollmap[move.to.y][move.to.x] = true;
            if (move.capt) {
                for (int n=0 ; n<NUM_DIRS ; n+=1) {
                    if (move.capts & (uint8_t(1) << n)) {
                        coord attack = move.to + dirs[n];
                        for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
                            piecestate dwarf = dwarfs[ix];
                            if (! dwarf.alive) continue;
                            if (dwarf.pos == attack) {
                                dwarf.alive = false;
                                dwarf.pos = coord(-1,-1);
                                dwarfmap[attack.y][attack.x] = false;
                                numdwarfs -= 1;
                            }
                        }
                    }
                }
            }

            calculate_dwarfmobility();
            calculate_dwarfthreats();
            calculate_trollthreats();

            break;
        }
    }
}
