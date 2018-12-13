#include "lachter.hh"

using namespace std;

bool inbounds(coord pos) {
    return 0 <= pos.x && pos.x < SIZE && 0 <= pos.y && pos.y < SIZE;
}

bool gamestate::legal() {
    boardmap occupied = blocks;

    // all pieces at legal coords
    // no pieces at same coords
    int dwarfcount = 0;
    for (int ix=0 ; ix<MAX_DWARFS ; ix+=1) {
        piecestate dwarf = dwarfs[ix];
        if (! dwarf.alive) continue;
        coord pos = dwarf.pos;
        if (! inbounds(pos)) return false;
        if (occupied[pos.y][pos.x]) return false;
        occupied[pos.y][pos.x] = true;
        dwarfcount += 1;
    }

    int trollcount = 0;
    for (int ix=0 ; ix<MAX_TROLLS ; ix+=1) {
        piecestate troll = trolls[ix];
        if (! troll.alive) continue;
        coord pos = troll.pos;
        if (! inbounds(pos)) return false;
        if (occupied[pos.y][pos.x]) return false;
        occupied[pos.y][pos.x] = true;
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
    }

    // threat maps accurate
}

/*
vector<gamemove> gamestate::allmoves() {
    return vector<gamemove>();

    return moves;
}
*/
