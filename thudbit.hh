#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

const int SIZE = 15;

struct coord {
    int x;
    int y;

    coord();
    coord(int newx, int newy);

    friend coord operator*(coord pos, int scalar);
    friend coord operator*(int scalar, coord pos);
    friend coord operator+(coord pos, coord delta);
    friend coord operator-(coord pos, coord delta);
    friend bool operator==(coord left, coord right);
    friend bool operator!=(coord left, coord right);
    friend bool operator<(coord left, coord right);

    friend ostream & operator<<(ostream & out, const coord pos);
    friend istream & operator>>(istream & in, coord & pos);

    bool inbounds();
};

const coord NOWHERE(-1,-1);

const int NUM_DIRS = 8;

const array<coord,NUM_DIRS> dirs = {(coord){1,0},  {1,1},   {0,1},  {-1,1},
                                           {-1,0}, {-1,-1}, {0,-1}, {1,-1}};

struct gamemove {
    bool isdwarfmove = false;
    coord from = NOWHERE;
    coord to = NOWHERE;
    bool capt = false;
    uint8_t capts = 0;

    gamemove();
    gamemove(bool newisdwarfmove, coord newfrom, coord newto);
    gamemove(bool newisdwarfmove, coord newfrom, coord newto, bool newcapt);
    gamemove(bool newisdwarfmove, coord newfrom, coord newto, bool newcapt,
             uint8_t newcapts);

    friend bool operator==(gamemove left, gamemove right);
    friend bool operator!=(gamemove left, gamemove right);
    friend bool operator<(gamemove left, gamemove right);

    friend ostream & operator<<(ostream & out, const gamemove move);
    friend istream & operator>>(istream & in, gamemove & move);

    void show();
};

struct bitref {
    uint16_t * bitline;
    uint8_t whichbit;

    bitref(uint16_t * newbitline, uint8_t newwhichbit);

    operator bool() const;
    bitref & operator=(bool newbit);
    //bitref & operator=(const bitref & newbit);
    //bool operator~() const;
};

using bitboard_array = array<uint16_t,SIZE>;
struct bitboard {
    bitboard_array data;

    bitboard();
    bitboard(bitboard_array newdata);

    bool operator[](coord pos) const;
    bitref operator[](coord pos);
    bool operator!=(bitboard & that);
};

const uint16_t LEFT_COLUMN_BIT = 0b0100000000000000;

struct gameboard {
    bitboard dwarfs;
    bitboard trolls;
    bitboard blocks;
};

const bitboard defaultblocks({
    0b111110000011111,
    0b111100000001111,
    0b111000000000111,
    0b110000000000011,
    0b100000000000001,
    0b000000000000000,
    0b000000000000000,
    0b000000010000000,
    0b000000000000000,
    0b000000000000000,
    0b100000000000001,
    0b110000000000011,
    0b111000000000111,
    0b111100000001111,
    0b111110000011111
});

const bitboard defaultdwarfs({
    0b000001101100000,
    0b000010000010000,
    0b000100000001000,
    0b001000000000100,
    0b010000000000010,
    0b100000000000001,
    0b100000000000001,
    0b000000000000000,
    0b100000000000001,
    0b100000000000001,
    0b010000000000010,
    0b001000000000100,
    0b000100000001000,
    0b000010000010000,
    0b000001101100000
});

const bitboard defaulttrolls({
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000111000000,
    0b000000101000000,
    0b000000111000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000,
    0b000000000000000
});

uint8_t neighborbits(bitboard board, coord pos);

const int MAX_DWARFS = 32;
const int MAX_TROLLS = 8;

typedef array<array<uint64_t,SIZE>,SIZE> boardhash_array;
struct zobrist_hashes {
    uint64_t turn;
    boardhash_array dwarfs;
    boardhash_array trolls;
};

struct gamestate {
    bool isdwarfturn;
    int sincecapt;

    int numdwarfs;
    int numtrolls;
    gameboard board;

    uint64_t hash;

    static zobrist_hashes hashes;
    static zobrist_hashes gen_hashes();

    gamestate();

    bool valid();

    vector<gamemove> allmoves();
    vector<gamemove> alldwarfmoves();
    vector<gamemove> alltrollmoves();

    bool validmove(gamemove move);
    void domove(gamemove move);
    void dodwarfmove(gamemove move);
    void dotrollmove(gamemove move);

    gamestate child(gamemove move);

    bool gameover();

    double final_score();
    double heuristic_score();
};
