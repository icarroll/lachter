#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

struct coord {
    int x;
    int y;

    coord(int newx, int newy);

    friend coord operator*(coord pos, int scalar);
    friend coord operator*(int scalar, coord pos);
    friend coord operator+(coord pos, coord delta);
    friend coord operator-(coord pos, coord delta);

    bool inbounds();
};

const int NUM_DIRS = 8;

const array<coord,NUM_DIRS> dirs = {(coord){1,0},  {1,1},   {0,1},  {-1,1},
                                           {-1,0}, {-1,-1}, {0,-1}, {1,-1}};

struct piecestate {
    bool alive;
    coord pos;
    bool inthreat;

    piecestate();
    piecestate(bool newalive, coord newpos, bool newinthreat);
};

const int SIZE = 15;

typedef array<array<bool,SIZE>,SIZE> boardmap;

boardmap operator||(boardmap left, boardmap right);

const boardmap blocks = {(array<bool,SIZE>)
        {1,1,1,1,1,0,0,0,0,0,1,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,0,0,0,0,0,1,1,1,1,1}
};

const boardmap dwarfstart = {(array<bool,SIZE>)
        {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
        {0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
        {0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
        {0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,0,0,0,0,0}
};

const boardmap trollstart = {(array<bool,SIZE>)
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,0,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

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
    boardmap dwarfmap;
    boardmap trollmap;
    array<array<int,NUM_DIRS>,MAX_DWARFS> dwarfmobility;
    boardmap dwarfthreats;
    boardmap trollthreats;

    gamestate();
    void calculate_maps();
    void calculate_dwarfmobility();
    void calculate_dwarfthreats();
    void calculate_trollthreats();

    bool valid();
    //vector<gamemove> allmoves();
};
