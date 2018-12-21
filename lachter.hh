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
    friend bool operator==(coord left, coord right);
    friend bool operator!=(coord left, coord right);

    bool inbounds();
};

const coord NOWHERE(-1,-1);

const int NUM_DIRS = 8;

const array<coord,NUM_DIRS> dirs = {(coord){1,0},  {1,1},   {0,1},  {-1,1},
                                           {-1,0}, {-1,-1}, {0,-1}, {1,-1}};

struct piecestate {
    bool alive;
    coord pos;

    piecestate();
    piecestate(coord newpos);
};

const int SIZE = 15;

typedef array<array<int,SIZE>,SIZE> boardmap_array;

struct boardmap {
    boardmap_array data;

    boardmap();

    bool operator[](coord pos) const;

    int & which(coord pos);

    bool operator!=(boardmap & that);
};

const int NOBODY = -1;

typedef array<array<bool,SIZE>,SIZE> boardflags_array;

struct boardflags {
    boardflags_array data;

    boardflags();
    boardflags(boardflags_array newdata);

    bool & operator[](coord pos);
    bool operator[](coord pos) const;

    bool operator!=(boardflags & that);
};

uint8_t neighborbits(boardmap map, coord pos);

const boardflags blocks({(array<bool,SIZE>)
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
});

const boardflags dwarfstart({(array<bool,SIZE>)
        {0,0,0,0,0,1,1,0,1,1,0,0,0,0,0},
        {0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
        {0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,1,0},
        {0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},
        {0,0,0,1,0,0,0,0,0,0,0,1,0,0,0},
        {0,0,0,0,1,0,0,0,0,0,1,0,0,0,0},
        {0,0,0,0,0,1,1,0,1,1,0,0,0,0,0}
});

const boardflags trollstart({(array<bool,SIZE>)
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
});

struct gamemove {
    bool isdwarfmove;
    coord from;
    coord to;
    bool capt;
    uint8_t capts;

    gamemove(bool newisdwarfmove, coord newfrom, coord newto);
    gamemove(bool newisdwarfmove, coord newfrom, coord newto, bool newcapt);
    gamemove(bool newisdwarfmove, coord newfrom, coord newto, bool newcapt,
             uint8_t newcapts);

    friend bool operator==(gamemove left, gamemove right);
    friend bool operator!=(gamemove left, gamemove right);
};

const int MAX_DWARFS = 32;
const int MAX_TROLLS = 8;

struct gamestate {
    bool isdwarfturn;
    int sincecapt;

    int numdwarfs;
    array<piecestate,MAX_DWARFS> dwarfs;
    boardmap dwarfmap;
    array<array<int,NUM_DIRS>,MAX_DWARFS> dwarfmobility;

    int numtrolls;
    array<piecestate,MAX_TROLLS> trolls;
    boardmap trollmap;

    boardflags dwarfthreats;
    boardflags trollthreats;

    gamestate();
    void calculate_dwarfmobility();
    void calculate_dwarfthreats();
    void calculate_trollthreats();

    bool valid();

    vector<gamemove> allmoves();
    vector<gamemove> alldwarfmoves();
    vector<gamemove> alltrollmoves();

    bool validmove(gamemove move);
    void domove(gamemove move);
    void dodwarfmove(gamemove move);
    void dotrollmove(gamemove move);

    bool gameover();

    float final_score();
    float heuristic_score();
};
