#include "lachter.hh"

using namespace std;

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

bool coord::inbounds() {
    return 0 <= x && x < SIZE && 0 <= y && y < SIZE;
}

bool gamestate::valid() {
    boardmap occupied = blocks;
    boardmap dwarfmap = {};
    boardmap trollmap = {};

    // all pieces at unique legal coords
    int dwarfcount = 0;
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (! pos.inbounds()) return false;
        if (occupied[pos.y][pos.x]) return false;
        occupied[pos.y][pos.x] = true;
        dwarfmap[pos.y][pos.x] = true;
        dwarfcount += 1;
    }

    int trollcount = 0;
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (! pos.inbounds()) return false;
        if (occupied[pos.y][pos.x]) return false;
        occupied[pos.y][pos.x] = true;
        trollmap[pos.y][pos.x] = true;
        trollcount += 1;
    }

    // piece counts accurate
    if (dwarfcount != numdwarfs || trollcount != numtrolls) return false;

    // dwarf mobility accurate
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

/*
vector<gamemove> gamestate::allmoves() {
    return vector<gamemove>();

    return moves;
}
*/
