#include <stdio.h>
#include "psqt.h"
#include "score.h"

int pawnPST[64] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        40, 54, 15, 33, 14, 22, 20, -13,
        -28, -45, -36, -10, 18, 100, 17, 20,
        -33, -38, -33, -22, 0, 11, -14, -40,
        -38, -40, -30, -24, -23, 7, 0, -33,
        -37, -30, -44, -30, -20, -8, 22, -17,
        -36, -29, -26, -41, -39, 11, 32, -12,
        0, 0, 0, 0, 0, 0, 0, 0,
};


int knightPST[64] = {
        -76, 2, -71, 53, 10, 7, 43, -118,
        -60, -75, -18, 32, 80, 83, -71, -13,
        -64, 9, -30, 45, 98, 93, 72, 53,
        -8, -28, -17, 3, -7, 35, 6, 53,
        -55, -35, -20, -12, -10, -9, 16, -5,
        -54, -70, -63, -57, -35, -49, -13, -46,
        -58, -100, -44, -34, -43, -55, -42, -38,
        -89, -70, -40, -61, -47, -57, -69, -133,
};

int bishopPST[64] = {
        -31, -80, -50, -48, -80, -93, 73, 0,
        -71, -66, -41, -25, 3, -27, 6, -17,
        -37, -14, -15, 19, 6, 70, 27, 16,
        -80, -25, -12, 27, 17, 31, -18, -2,
        -56, -40, -25, 2, 7, -20, -21, 5,
        -21, -29, -27, -9, -22, -11, -11, 1,
        -14, -32, 18, -31, -30, -7, 3, -6,
        -11, 7, -32, -28, -42, -49, -58, -37,
};

int rookPST[64] = {
        50, 56, 0, 34, 82, 22, 83, 69,
        -14, 0, 20, 81, 93, 92, 85, 88,
        -43, -20, -10, 24, 61, 83, 83, 38,
        -70, -46, -31, -14, -9, 1, 59, 22,
        -88, -83, -63, -57, -37, -62, 20, -8,
        -85, -75, -68, -54, -45, -35, -2, -30,
        -82, -80, -68, -56, -37, -31, -7, -65,
        -64, -52, -45, -40, -30, -39, -9, -54,
};

int queenPST[64] = {
        14, -3, -23, 23, 78, 73, 73, 63,
        -54, -82, -34, -27, 53, 55, 81, 73,
        -41, -55, -20, -44, 26, 88, 83, 54,
        -58, -56, -58, -48, -8, -6, 40, -1,
        -47, -69, -51, -45, -37, -31, -13, 1,
        -67, -55, -52, -48, -31, -28, -23, -2,
        -50, -50, -35, -23, -20, -11, 3, 4,
        -94, -73, -34, -17, -16, -93, -93, -9,
};

int kingPST[64] = {
        53, 23, -89, -89, -35, 36, 43, 34,
        -65, 35, -33, 1, 33, 29, 5, -99,
        -8, -33, -4, 9, 27, 43, 43, -41,
        -93, -29, -31, -58, -43, -39, -34, -73,
        -32, -60, -37, -106, -74, -31, -51, -103,
        -93, -7, -39, -77, -79, -58, -23, -30,
        -27, -47, -39, -71, -51, -30, 42, 62,
        -35, 24, 9, -83, 27, -45, 77, 74,
};

int egPawnPST[64] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        133, 116, 122, 85, 42, 88, 133, 110,
        83, 80, 60, 24, -5, -4, 43, 53,
        44, 38, 18, 0, -11, -3, 24, 20,
        30, 23, 10, -4, -9, -8, 6, 3,
        17, 14, 7, -3, -7, 0, -3, -3,
        20, 24, 14, 2, 18, 16, 6, -12,
        7, 0, 0, 0, 0, 0, 0, 0,
};


int egKnightPST[64] = {
        -16, 43, 39, 15, 13, 39, 24, 33,
        14, 44, 29, 37, 3, -11, 35, 13,
        14, 12, 49, 20, 20, 25, 20, 2,
        19, 12, 43, 49, 61, 40, 24, 22,
        13, 23, 32, 43, 53, 36, 17, 24,
        -45, 3, 7, 44, 27, 19, 4, -13,
        11, 27, -14, -2, 5, -8, -6, 10,
        -15, -40, -16, -12, 1, -11, 9, -49,
};

int egBishopPST[64] = {
        0, 30, 34, 28, 16, 20, 11, 32,
        30, 40, 11, -7, 7, 30, 1, 7,
        21, 25, 14, 4, 7, 20, 23, 12,
        29, 15, 14, 21, 12, 7, 18, 21,
        -9, 14, 24, 23, 19, 10, 24, 3,
        -21, -14, 26, 18, 21, 8, -17, -12,
        4, -6, -8, -6, 4, 0, -12, -13,
        3, -30, -24, -6, -2, -6, -23, 0,
};

int egRookPST[64] = {
        43, 44, 68, 54, 37, 68, 58, 49,
        85, 75, 75, 49, 46, 50, 46, 46,
        78, 76, 76, 58, 46, 54, 51, 43,
        78, 79, 74, 55, 53, 58, 57, 43,
        63, 71, 65, 50, 37, 59, 34, 33,
        36, 30, 40, 22, 18, 21, 2, 16,
        28, 43, 31, 27, 9, 15, -10, 8,
        30, 19, 26, 20, 12, 27, 12, 16,
};

int egQueenPST[64] = {
        50, 73, 73, 78, 78, 73, 73, 63,
        69, 34, 83, 83, 83, 83, 43, 73,
        46, 66, 78, 88, 77, 88, 83, 73,
        75, 65, 88, 88, 88, 88, 77, 78,
        16, 56, 56, 65, 73, 71, 53, 47,
        25, 19, 43, 26, 17, 24, 31, 8,
        35, 9, -7, -39, -7, -33, -83, -61,
        14, 5, -51, -45, -88, -68, -77, -45,
};

int egKingPST[64] = {
        -37, 31, 24, 47, 61, 67, 13, -53,
        39, 56, 74, 76, 83, 83, 83, 39,
        24, 78, 96, 98, 98, 98, 83, 52,
        20, 69, 87, 91, 94, 84, 67, 37,
        -11, 39, 49, 76, 63, 47, 29, 3,
        -9, 3, 27, 41, 42, 27, 2, -30,
        -38, -6, 0, 22, 16, 7, -25, -51,
        -62, -51, -34, -20, -73, -27, -65, -93,
};

void initPSQT() {
    for (int stage = 0; stage < 99; stage++) {
        for (int sq = 0; sq < 64; sq++) {
            PST[stage][PAWN][sq] = getScore2(pawnPST[sq], egPawnPST[sq], stage);
            PST[stage][KNIGHT][sq] = getScore2(knightPST[sq], egKnightPST[sq], stage);
            PST[stage][BISHOP][sq] = getScore2(bishopPST[sq], egBishopPST[sq], stage);
            PST[stage][ROOK][sq] = getScore2(rookPST[sq], egRookPST[sq], stage);
            PST[stage][QUEEN][sq] = getScore2(queenPST[sq], egQueenPST[sq], stage);
            PST[stage][KING][sq] = getScore2(kingPST[sq], egKingPST[sq], stage);
        }
    }
}