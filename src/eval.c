#include "eval.h"
#include "score.h"
#include <math.h>

//Material
int PAWN_EV_MG = 100;
int KNIGHT_EV_MG = 311;
int BISHOP_EV_MG = 264;
int ROOK_EV_MG = 417;
int QUEEN_EV_MG = 1005;

int PAWN_EV_EG = 159;
int KNIGHT_EV_EG = 405;
int BISHOP_EV_EG = 427;
int ROOK_EV_EG = 643;
int QUEEN_EV_EG = 1133;

//Mobility bonuses
int QueenMobility[28] = {
        -28, -20, -10, -1, 35, 34, 64, 70, 75, 83, 88, 100, 104, 113, 116, 122, 124, 125, 141, 134, 148, 146, 155, 156, 158, 173, 174, 175,
};
int RookMobility[15] = {-86, -48, -6, 0, 5, 13, 20, 26, 29, 37, 43, 45, 45, 44, 43, };
int BishopMobility[14] = {-65, -55, -15, 2, 16, 26, 34, 42, 49, 56, 59, 61, 57, 69, };
int KnightMobility[8] = {-153, -72, -46, -24, -7, 4, 13, 23, };

//additional bonuses and penalties
int PassedPawnBonus[8] = {0, -11, -10, 4, 35, 88, 127, 0, };
int DoublePawnsPenalty = -33;
int IsolatedPawnPenalty = -8;
int RookOnOpenFileBonus = 30;
int RookOnPartOpenFileBonus = 26;
int KingDangerFactor = 733;
int DoubleBishopsBonusMG = 32;
int DoubleBishopsBonusEG = 78;

int DoubleBishopsBonus() {
    return getScore2(DoubleBishopsBonusMG, DoubleBishopsBonusEG, stage);
}

int fullEval(Board *board) {
    int eval = board->eval;
    stage = stageGame(board);

    eval += psqtEval(board);

    //Material Eval
    eval += materialEval(board);

    //Mobility eval
    eval += (mobilityAndKingDangerEval(board, WHITE) - mobilityAndKingDangerEval(board, BLACK));

    //Pieces eval
    eval += (pawnsEval(board, WHITE) - pawnsEval(board, BLACK));
    eval += bishopsEval(board);
    eval += (rooksEval(board, WHITE) - rooksEval(board, BLACK));

    //King safety
    // eval += (kingEval(board, WHITE) - kingEval(board, BLACK));

    return (board->color == WHITE ? eval : -eval);
}


int materialEval(Board* board) {
    int eval = 0;

    int wpCount = popcount(board->pieces[PAWN] & board->colours[WHITE]);
    int bpCount = popcount(board->pieces[PAWN] & board->colours[BLACK]);
    int wnCount = popcount(board->pieces[KNIGHT] & board->colours[WHITE]);
    int bnCount = popcount(board->pieces[KNIGHT] & board->colours[BLACK]);
    int wbCount = popcount(board->pieces[BISHOP] & board->colours[WHITE]);
    int bbCount = popcount(board->pieces[BISHOP] & board->colours[BLACK]);
    int wrCount = popcount(board->pieces[ROOK] & board->colours[WHITE]);
    int brCount = popcount(board->pieces[ROOK] & board->colours[BLACK]);
    int wqCount = popcount(board->pieces[QUEEN] & board->colours[WHITE]);
    int bqCount = popcount(board->pieces[QUEEN] & board->colours[BLACK]);

    eval += PAWN_EVAL[stage] * (wpCount - bpCount);

    eval += KNIGHT_EVAL[stage] * (wnCount - bnCount);
    eval += BISHOP_EVAL[stage] * (wbCount - bbCount);
    eval += ROOK_EVAL[stage] * (wrCount - brCount);
    eval += QUEEN_EVAL[stage] * (wqCount - bqCount);

    return eval;
}

int psqtEval(Board* board) {
    int eval = 0;

    U64 mask = board->pieces[PAWN];
    eval += psqtPieceEval(board, mask, PAWN);

    mask = board->pieces[KNIGHT];
    eval += psqtPieceEval(board, mask, KNIGHT);

    mask = board->pieces[BISHOP];
    eval += psqtPieceEval(board, mask, BISHOP);

    mask = board->pieces[ROOK];
    eval += psqtPieceEval(board, mask, ROOK);

    mask = board->pieces[QUEEN];
    eval += psqtPieceEval(board, mask, QUEEN);

    mask = board->pieces[KING];
    eval += psqtPieceEval(board, mask, KING);

    return eval;
}

int psqtPieceEval(Board *board, U64 mask, int pieceType) {
    int eval = 0;

    while (mask) {
        int sq = firstOne(mask);
        U64 isWhite = squareBitboard[sq] & board->colours[WHITE];
        int relativeSq = isWhite ? square(7 - rankOf(sq), fileOf(sq)) : sq;
        int multiple = isWhite ? 1 : -1;
        int sqEval = multiple * PST[stage][pieceType][relativeSq];

        eval += sqEval;

        clearBit(&mask, sq);
    }

    return eval;
}

int kingDanger(int attacksCount) {
    double normalized = (attacksCount / 100. * 10.) - 5;

    return KingDangerFactor * (1. / (1. + exp(-normalized)));
}

int mobilityAndKingDangerEval(Board *board, int color) {
    int eval = 0;

    U64 our = board->colours[color]; //our pieces
    U64 enemy = board->colours[!color]; //enemy pieces
    U64 occu = (our | enemy); //all pieces

    U64 mask = board->pieces[PAWN] & enemy;
    U64 pAttacks;

    if (!color == WHITE)
        pAttacks = ((mask << 9) & ~files[0]) | ((mask << 7) & ~files[7]);
    else
        pAttacks = ((mask >> 9) & ~files[7]) | ((mask >> 7) & ~files[0]);

    U64 possibleSq = ~pAttacks;


    int enemyKingPos = firstOne(enemy & board->pieces[KING]);
    U64 enemyKingDangerCells = kingAttacks[enemyKingPos] & ~enemy;

    int kingDangerValue = 0;

    //Rooks mobility
    mask = board->pieces[ROOK] & our;

    while (mask) {
        int from = firstOne(mask);
        U64 possibleMoves = rookPossibleMoves[from][getMagicIndex(occu & rookMagicMask[from] & unSquareBitboard[from],
                                                                  rookMagic[from], rookPossibleMovesSize[from])];
        eval += RookMobility[popcount(possibleMoves & possibleSq)];

        kingDangerValue += 3 * popcount(possibleMoves & enemyKingDangerCells);

        clearBit(&mask, from);
    }

    //Bishops mobility
    mask = board->pieces[BISHOP] & our;

    while (mask) {
        int from = firstOne(mask);
        U64 possibleMoves = bishopPossibleMoves[from][getMagicIndex(
                occu & bishopMagicMask[from] & unSquareBitboard[from], bishopMagic[from],
                bishopPossibleMovesSize[from])];
        eval += BishopMobility[popcount(possibleMoves & possibleSq)];

        kingDangerValue += 2 * popcount(possibleMoves & enemyKingDangerCells);

        clearBit(&mask, from);
    }

    //Queens mobility
    mask = board->pieces[QUEEN] & our;

    while (mask) {
        int from = firstOne(mask);
        U64 possibleMoves = (
                (rookPossibleMoves[from][getMagicIndex(occu & rookMagicMask[from] & unSquareBitboard[from],
                                                       rookMagic[from], rookPossibleMovesSize[from])])
                | (bishopPossibleMoves[from][getMagicIndex(occu & bishopMagicMask[from] & unSquareBitboard[from],
                                                           bishopMagic[from], bishopPossibleMovesSize[from])])
        );
        eval += QueenMobility[popcount(possibleMoves & possibleSq)];

        kingDangerValue += 5 * popcount(possibleMoves & enemyKingDangerCells);

        clearBit(&mask, from);
    }

    //Knights mobility
    mask = board->pieces[KNIGHT] & our;

    while (mask) {
        int from = firstOne(mask);
        U64 possibleMoves = knightAttacks[from];
        eval += KnightMobility[popcount(possibleMoves & possibleSq)];

        kingDangerValue += 2 * popcount(possibleMoves & enemyKingDangerCells);

        clearBit(&mask, from);
    }

    return eval + KingDanger[kingDangerValue];
}

int pawnsEval(Board *board, int color) {
    int eval = 0;

    U64 ourPawns = board->colours[color] & board->pieces[PAWN];
    U64 enemyPawns = board->colours[!color] & board->pieces[PAWN];

    //isolated pawn bonus
    eval += (IsolatedPawnsHash[horizontalScan(ourPawns)]);

    //double pawns bonus
    for (int f = 0; f < 8; ++f)
        eval += DoublePawnsPenalty * (popcount(ourPawns & files[f]) > 1);

    //passed pawn bonus
    while (ourPawns) {
        int sq = firstOne(ourPawns);
        if (color == WHITE) {
            if (!(plus8[sq] & enemyPawns))
                eval += getPassedPawnBonus(sq, color);
        } else {
            if (!(minus8[sq] & enemyPawns))
                eval += getPassedPawnBonus(sq, color);
        }
        clearBit(&ourPawns, sq);
    }

    return eval;
}

int bishopsEval(Board *board) {
    int eval = 0;
    int score = DoubleBishopsBonus();

    //double bishops bonus
    eval += (score * (popcount(board->pieces[BISHOP] & board->colours[WHITE]) > 1) -
             score * (popcount(board->pieces[BISHOP] & board->colours[BLACK]) > 1));

    return eval;
}

int getPassedPawnBonus(int sq, int color) {
    if (color == WHITE)
        return PassedPawnBonus[rankOf(sq)];

    return PassedPawnBonus[7 - rankOf(sq)];
}

int kingEval(Board *board, int color) {
    int eval = 0;
    U64 enemy = board->colours[!color];
    int kingPos = firstOne(board->pieces[KING] & board->colours[color]);

    while (enemy) {
        int sq = firstOne(enemy);
        eval -= 4 * distanceBonus[sq][kingPos] * (pieceType(board->squares[sq]) == QUEEN);
        eval -= 3 * distanceBonus[sq][kingPos] * (pieceType(board->squares[sq]) == ROOK);
        eval -= 2 * distanceBonus[sq][kingPos] * (pieceType(board->squares[sq]) == KNIGHT);
        eval -= 2 * distanceBonus[sq][kingPos] * (pieceType(board->squares[sq]) == BISHOP);
        clearBit(&enemy, sq);
    }

    return eval;
}

int mateScore(int eval) {
    return (eval >= MATE_SCORE - 100 || eval <= -MATE_SCORE + 100);
}

void initEval() {
    PAWN_EVAL = (int*) malloc(sizeof(int) * STAGE_N);
    KNIGHT_EVAL = (int*) malloc(sizeof(int) * STAGE_N);
    BISHOP_EVAL = (int*) malloc(sizeof(int) * STAGE_N);
    ROOK_EVAL = (int*) malloc(sizeof(int) * STAGE_N);
    QUEEN_EVAL = (int*) malloc(sizeof(int) * STAGE_N);

    initDependencyEval();
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j)
            distanceBonus[i][j] = 14 - (abs(rankOf(i) - rankOf(j)) + abs(fileOf(i) - fileOf(j)));
    }
}

void destroyEval() {
    free(PAWN_EVAL);
    free(KNIGHT_EVAL);
    free(BISHOP_EVAL);
    free(ROOK_EVAL);
    free(QUEEN_EVAL);
}

void initDependencyEval() {
    initPSQT();

    for (int i = 0; i < STAGE_N; i++) {
        PAWN_EVAL[i] = getScore2(PAWN_EV_MG, PAWN_EV_EG, i);
        KNIGHT_EVAL[i] = getScore2(KNIGHT_EV_MG, KNIGHT_EV_EG, i);
        BISHOP_EVAL[i] = getScore2(BISHOP_EV_MG, BISHOP_EV_EG, i);
        ROOK_EVAL[i] = getScore2(ROOK_EV_MG, ROOK_EV_EG, i);
        QUEEN_EVAL[i] = getScore2(QUEEN_EV_MG, QUEEN_EV_EG, i);
    }

    for (int i = 0; i < 100; i++) {
        KingDanger[i] = kingDanger(i);
    }

    //Isolated pawn hash init
    for (int i = 0; i < 256; ++i) {
        IsolatedPawnsHash[i] = 0;
        for (int f = 0; f < 8; ++f) {
            int leftEmpty = 1, rightEmpty = 1;

            if (f < 7)
                rightEmpty = !getBit8(i, f + 1);
            if (f > 0)
                leftEmpty = !getBit8(i, f - 1);

            IsolatedPawnsHash[i] += IsolatedPawnPenalty * (leftEmpty && rightEmpty && getBit(i, f));
        }
    }
}

void initStagedPSQT(int st) {
    for (int sq = 0; sq < 64; sq++) {
        PST[st][PAWN][sq] = getScore2(pawnPST[sq], egPawnPST[sq], st);
        PST[st][KNIGHT][sq] = getScore2(knightPST[sq], egKnightPST[sq], st);
        PST[st][BISHOP][sq] = getScore2(bishopPST[sq], egBishopPST[sq], st);
        PST[st][ROOK][sq] = getScore2(rookPST[sq], egRookPST[sq], st);
        PST[st][QUEEN][sq] = getScore2(queenPST[sq], egQueenPST[sq], st);
        PST[st][KING][sq] = getScore2(kingPST[sq], egKingPST[sq], st);
    }
}

void initDependencyStagedEval(int st) {
    initStagedPSQT(st);

    for (int i = 0; i < 100; i++) {
        KingDanger[i] = kingDanger(i);
    }

    PAWN_EVAL[stage] = getScore2(PAWN_EV_MG, PAWN_EV_EG, st);
    KNIGHT_EVAL[stage] = getScore2(KNIGHT_EV_MG, KNIGHT_EV_EG, st);
    BISHOP_EVAL[stage] = getScore2(BISHOP_EV_MG, BISHOP_EV_EG, st);
    ROOK_EVAL[stage] = getScore2(ROOK_EV_MG, ROOK_EV_EG, st);
    QUEEN_EVAL[stage] = getScore2(QUEEN_EV_MG, QUEEN_EV_EG, st);

    //Isolated pawn hash init
    for (int i = 0; i < 256; ++i) {
        IsolatedPawnsHash[i] = 0;
        for (int f = 0; f < 8; ++f) {
            int leftEmpty = 1, rightEmpty = 1;

            if (f < 7)
                rightEmpty = !getBit8(i, f + 1);
            if (f > 0)
                leftEmpty = !getBit8(i, f - 1);

            IsolatedPawnsHash[i] += IsolatedPawnPenalty * (leftEmpty && rightEmpty && getBit(i, f));
        }
    }
}

int pVal(Board* b, int n) {
    int stage = stageGame(b);

    switch (n) {
        case 0:
            return 0;
        case 1:
            return PAWN_EVAL[stage];
        case 2:
            return KNIGHT_EVAL[stage];
        case 3:
            return BISHOP_EVAL[stage];
        case 4:
            return ROOK_EVAL[stage];
        case 5:
            return QUEEN_EVAL[stage];
        case 6:
            return 0;
    }
}

int stageGame(Board *board) {
    return popcount(board->pieces[QUEEN]) * 12 + popcount(board->pieces[ROOK]) * 8 +
           popcount(board->pieces[BISHOP]) * 5 + popcount(board->pieces[KNIGHT]) * 5;
}

int rooksEval(Board *board, int color) {
    int eval = 0;
    U64 our = board->colours[color];

    U64 rookMask = board->pieces[ROOK] & our;

    //rook on open file bonus

    while (rookMask) {
        int sq = firstOne(rookMask);

        U64 file = files[fileOf(sq)];

        int ourPawnsOnFile = popcount(file & board->pieces[PAWN] & board->colours[color]) > 0;
        int enemyPawnsOnFile = popcount(file & board->pieces[PAWN] & board->colours[!color]) > 0;

        if (!ourPawnsOnFile) {
            if (enemyPawnsOnFile) {
                eval += RookOnPartOpenFileBonus;
            } else {
                eval += RookOnOpenFileBonus;
            }
        }

        clearBit(&rookMask, sq);
    }

    return eval;
}

U8 horizontalScan(U64 bitboard) {
    return (!!(bitboard & files[0])) | (!!(bitboard & files[1]) << 1)
           | (!!(bitboard & files[2])) << 2 | (!!(bitboard & files[3]) << 3)
           | (!!(bitboard & files[4])) << 4 | (!!(bitboard & files[5]) << 5)
           | (!!(bitboard & files[6])) << 6 | (!!(bitboard & files[7]) << 7);
}