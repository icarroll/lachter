#pragma once

struct piecestate {
    bool alive;
    int x;
    int y;
    bool inthreat;
};

const int MAX_DWARFS = 32;
const int MAX_TROLLS = 8;
const int NUM_DIRS = 8;
const int SIZE = 15;

typedef bool threatmap[SIZE][SIZE];

struct gamestate {
    bool isdwarfturn;
    piecestate dwarfs[MAX_DWARFS];
    piecestate trolls[MAX_TROLLS];
    int numdwarfs;
    int numtrolls;
    int mobility[MAX_DWARFS][NUM_DIRS];
    threatmap dwarfthreats;
    threatmap trollthreats;
};
