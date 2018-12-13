#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace std;

struct coord {
    int x;
    int y;
};

const int NUM_DIRS = 8;

array<coord,NUM_DIRS> dirs = {(coord){1,0},  {1,1},   {0,1},  {-1,1},
                                     {-1,0}, {-1,-1}, {0,-1}, {1,-1}};

struct piecestate {
    bool alive;
    coord pos;
    bool inthreat;
};

const int SIZE = 15;

typedef array<array<bool,SIZE>,SIZE> boardmap;

const boardmap blocks = {1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,
                         1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
                         1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,
                         1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,
                         1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
                         1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,
                         1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,
                         1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
                         1,1,1,1,1,0,0,0,0,0,1,1,1,1,1};

struct gamemove {
    bool isdwarfmove;
    coord from;
    coord to;
    bool capt;
    uint8_t capts;
};

const int MAX_DWARFS = 32;
const int MAX_TROLLS = 8;

struct gamestate {
    bool isdwarfturn;
    array<piecestate,MAX_DWARFS> dwarfs;
    array<piecestate,MAX_TROLLS> trolls;
    int numdwarfs;
    int numtrolls;
    array<array<int,NUM_DIRS>,MAX_DWARFS> dwarfmobility;
    boardmap dwarfthreats;
    boardmap trollthreats;

    bool legal();
    //vector<gamemove> allmoves();
};
