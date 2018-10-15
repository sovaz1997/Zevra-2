#ifndef SEARCH_H
#define SEARCH_H

#include <math.h>
#include <time.h>
#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "uci.h"
#include "timemanager.h"
#include "transposition.h"

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

struct SearchInfo {
    U64 nodesCount;
    U16 bestMove;
    Timer timer;
    TimeManager tm;
    int abort;
    U16 killer[2][MAX_PLY + 1];
    int history[64][64];
    int nullMoveSearch;
};

//Тип оценки
enum {
    empty = 0,
    lowerbound = 1,
    upperbound = 2,
    exact = 3
};

U16 moves[MAX_PLY][256];
int movePrice[256];
int mvvLvaScores[7][7];
int lmr[MAX_PLY][64];

void iterativeDeeping(Board* board, TimeManager tm);
int search(Board* board, SearchInfo* searchInfo, int alpha, int beta, int depth, int height);
int quiesceSearch(Board* board, SearchInfo* searchInfo, int alpha, int beta, int height);
U64 perftTest(Board* board, int depth, int height);
void perft(Board* board, int depth);
void moveOrdering(Board* board, U16* moves, SearchInfo* searchInfo, int height);
void sort(U16* moves, int count);
void initSearch();
void resetSearchInfo(SearchInfo* info, TimeManager tm);
void replaceTransposition(Transposition* tr, Transposition new_tr, int height);

#endif