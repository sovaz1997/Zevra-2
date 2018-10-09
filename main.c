#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitboards.h"
#include "board.h"
#include "movegen.h"
#include "magic.h"

void initEngine();

int main() {
    srand(time(0));
    
    
    initEngine();

    Board* board = (Board*) malloc(sizeof(Board));

    char startpos[] = "rnbqkbnr/pppppppp/8/8/8/8/1PPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    setFen(board, startpos);
    printBoard(board);

    uint16_t moveList[256];
    
    movegen(board, moveList);

    uint16_t* curMove = moveList;

    while(*curMove) {
        char move[6];
        moveToString(*curMove, move);
        printf("%s\n", move);
        ++curMove;
    }
    

    free(board);

    return 0;
}

void initEngine() {
    initBitboards();
    magicArraysInit();
}