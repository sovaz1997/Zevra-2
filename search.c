#include "search.h"

void* go(void* thread_data) {
    SearchArgs* args = (SearchArgs*)thread_data;
    iterativeDeeping(args->board, args->tm);
    SEARCH_COMPLETE = 1;
}

void iterativeDeeping(Board* board, TimeManager tm) {
    ++ttAge;
    SearchInfo searchInfo;
    char bestMove[6];

    resetSearchInfo(&searchInfo, tm);
    startTimer(&searchInfo.timer);
    int eval = 0;
    for(int i = 1; i <= tm.depth; ++i) {
        eval = aspirationWindow(board, &searchInfo, i, eval);
        moveToString(searchInfo.bestMove, bestMove);
        if(ABORT && i > 1) {
            break;
        }
    }


    printf("info nodes %llu time %llu\n", searchInfo.nodesCount, getTime(&searchInfo.timer));
    printf("bestmove %s\n", bestMove);
    fflush(stdout);
}

int aspirationWindow(Board* board, SearchInfo* searchInfo, int depth, int score) {
    int delta = 15;
    int alpha = max(-MATE_SCORE, score - delta);
    int beta = min(MATE_SCORE, score + delta);

    if(depth <= 5) {
        return search(board, searchInfo, -MATE_SCORE, MATE_SCORE, depth, 0);
    }

    char bestMove[6];

    int f = score;
    while(abs(f) < MATE_SCORE - 1) {
         f = search(board, searchInfo, alpha, beta, depth, 0);

        U64 searchTime = getTime(&searchInfo->timer);
        int speed = (searchTime < 1 ? 0 : (searchInfo->nodesCount / (searchTime / 1000.)));
        int hashfull = (double)ttFilledSize  / (double)ttSize * 1000;

        moveToString(searchInfo->bestMove, bestMove);

        if(ABORT) {
            break;
        }

        int evalType;

        if(f > alpha && f < beta) {
            evalType = exact;
        }

        if(f <= alpha) {
            beta = (alpha + beta) / 2;
            alpha = max(-MATE_SCORE, alpha - delta);
            evalType = upperbound;
        }

        if(f >= beta) {
            beta = min(MATE_SCORE, beta + delta);
            evalType = lowerbound;
        }

        printSearchInfo(searchInfo, board, depth, f, evalType);

        if(evalType == exact) {
            return f;
        }

        delta += delta / 2;
    }
}

int search(Board* board, SearchInfo* searchInfo, int alpha, int beta, int depth, int height) {
    ++searchInfo->nodesCount;
    if(ABORT) {
        return 0;
    }

    if(depth < 0 || depth > MAX_PLY - 1) {
        depth = 0;
    }

    //Mate Distance Pruning
    int mate_val = MATE_SCORE - height;
    if(mate_val < beta) {
        beta = mate_val;
        if(alpha >= mate_val) {
            return mate_val;
        }
    }

    mate_val = -MATE_SCORE + height;
    if(mate_val > alpha) {
        alpha = mate_val;
        if(beta <= mate_val) {
            return mate_val;
        }
    }

    U64 keyPosition = board->key;
    Transposition* ttEntry = &tt[keyPosition & ttIndex];

    int root = (height ? 0 : 1);
    int pvNode = (beta - alpha > 1);

    if(isDraw(board) && !root || ABORT) {
        return 0;
    }

    int weInCheck = !!(inCheck(board, board->color));

    if(depth >= 3 && testAbort(getTime(&searchInfo->timer), &searchInfo->tm)) {
        setAbort(1);
        return 0;
    }

    Undo undo;

    if(ttEntry->evalType && ttEntry->depth >= depth && !root && ttEntry->key == keyPosition) {
        int score = ttEntry->eval;
        if(score > MATE_SCORE - 100) {
            score -= height;
        } else if(score < -MATE_SCORE + 100) {
            score += height;
        }

        if(ttEntry->evalType == lowerbound && score >= beta && !mateScore(score)) {
            return score;
        } else if(ttEntry->evalType == upperbound && score <= alpha && !mateScore(score)) {
            return score;
        } else if(ttEntry->evalType == exact) {
            return score;
        }
    }

    if((depth <= 0 && !weInCheck) || height >= MAX_PLY - 1) {
        return quiesceSearch(board, searchInfo, alpha, beta, height);
    }

    //Null Move pruning
    
    int R = 2 + depth / 6;
    int staticEval = fullEval(board);
    
    int pieceCount = popcount(board->colours[WHITE] | board->colours[BLACK]);
    if(NullMovePruningAllow && pieceCount > 7 && !pvNode && haveNoPawnMaterial(board) && !weInCheck && !root && !searchInfo->nullMoveSearch && depth > R && (staticEval >= beta || depth <= 4)) {
        makeNullMove(board);
        searchInfo->nullMoveSearch = 1;

        int eval = -search(board, searchInfo, -beta, -beta + 1, depth - 1 - R, height + 1);

        searchInfo->nullMoveSearch = 0;
        unmakeNullMove(board);

        if(eval >= beta) {
            return beta;
        }
    }

    movegen(board, moves[height]);
    moveOrdering(board, moves[height], searchInfo, height, depth);

    U16* curMove = moves[height];

    int movesCount = 0, pseudoMovesCount = 0;

    Transposition new_tt;

    int oldAlpha = alpha;

    while(*curMove) {
        int nextDepth = depth - 1;
        ++pseudoMovesCount;
        makeMove(board, *curMove, &undo);

        if(inCheck(board, !board->color)) {
            unmakeMove(board, *curMove, &undo);
            ++curMove;
            continue;
        }
        
        ++movesCount;

        int extensions = inCheck(board, board->color) || MoveType(*curMove) == PROMOTION_MOVE;
        int goodMove = isKiller(searchInfo, board->color, *curMove, depth);
        int quiteMove = (!goodMove && !undo.capturedPiece && MoveType(*curMove) != ENPASSANT_MOVE);

        if(root && depth > 12) {
            char moveStr[6];
            moveToString(*curMove, moveStr);
            printf("info currmove %s currmovenumber %d\n", moveStr, movesCount);
            fflush(stdout);
        }

        //Fulility pruning
        if(depth < 7 && !goodMove && !extensions && !root && FutilityPruningAllow) {
            if(staticEval + FutilityMargin[depth] + pVal[pieceType(undo.capturedPiece)] <= alpha) {
                unmakeMove(board, *curMove, &undo);
                ++curMove;
                continue;
            }
        }

        int reductions = lmr[min(depth, MAX_PLY - 1)][min(63, movesCount)];
        int historyReduced = 0;

        //History pruning
        if(HistoryPruningAllow && !pvNode && !extensions && !goodMove && depth >= 7 && movePrice[height][pseudoMovesCount - 1] >= 0 && movePrice[height][pseudoMovesCount - 1] <= 20000) {
            --nextDepth;
            historyReduced = 1;
        }

        int eval;
        if(movesCount == 1) {
            eval = -search(board, searchInfo, -beta, -alpha, nextDepth + extensions, height + 1);
        } else {
            if(LmrPruningAllow && movesCount >= 3 && quiteMove) {
                eval = -search(board, searchInfo, -alpha - 1, -alpha, nextDepth + extensions - reductions, height + 1);
                if(eval > alpha) {
                    eval = -search(board, searchInfo, -beta, -alpha, nextDepth + extensions, height + 1);
                }
            } else {
                eval = -search(board, searchInfo, -alpha - 1, -alpha, nextDepth + extensions, height + 1);
    
                if(eval > alpha && eval < beta) {
                    eval = -search(board, searchInfo, -beta, -alpha, nextDepth + extensions, height + 1);
                }
            }
        }

        if(HistoryPruningAllow && historyReduced && eval >= beta) {
            ++nextDepth;
            eval = -search(board, searchInfo, -beta, -alpha, nextDepth + extensions, height + 1);
        }

        unmakeMove(board, *curMove, &undo);
        
        if(eval > alpha) {
            alpha = eval;
            if(root && !ABORT) {
                searchInfo->bestMove = *curMove;
            }

            setTransposition(&new_tt, keyPosition, alpha, (alpha >= beta ? lowerbound : exact), depth, *curMove, ttAge);
        }
        if(alpha >= beta) {
            if(!undo.capturedPiece) {
                if(searchInfo->killer[board->color][depth]) {
                    searchInfo->secondKiller[board->color][height] = searchInfo->killer[board->color][depth];
                }
                
                searchInfo->killer[board->color][depth] = *curMove;
                history[board->color][MoveFrom(*curMove)][MoveTo(*curMove)] += (depth * depth);
            }

            break;
        }
        ++curMove;
    }

    if(ABORT) {
        return 0;
    }

    if(oldAlpha == alpha) {
        setTransposition(&new_tt, keyPosition, alpha, upperbound, depth, 0, ttAge);
    }

    replaceTransposition(ttEntry, new_tt, height);

    if(!movesCount) {
        if(inCheck(board, board->color)) {
            return -MATE_SCORE + height;
        } else {
            return 0;
        }
    }

    return alpha;
}

int quiesceSearch(Board* board, SearchInfo* searchInfo, int alpha, int beta, int height) {
    searchInfo->selDepth = max(searchInfo->selDepth, height);
    if(height >= MAX_PLY - 1) {
        return fullEval(board);
    }
    
    if(testAbort(getTime(&searchInfo->timer), &searchInfo->tm)) {
        setAbort(1);
        return 0;
    }

    if(ABORT) {
        return 0;
    }
    
    int val = fullEval(board);
    if(val >= beta) {
        return beta;
    }

    int delta = QUEEN_EV;
    if(havePromotionPawn(board)) {
        delta += (QUEEN_EV - 200);
    }

    if(val < alpha - delta) {
        return val;
    }

    if(alpha < val) {
        alpha = val;
    }

    attackgen(board, moves[height]);
    moveOrdering(board, moves[height], searchInfo, height, 0);
    U16* curMove = moves[height];
    Undo undo;
    int pseudoMovesCount = 0;
    while(*curMove) {
        if(ABORT) {
            return 0;
        }

        if(movePrice[height][pseudoMovesCount] < 0) {
            break;
        }

        ++pseudoMovesCount;

        makeMove(board, *curMove, &undo);
    
        if(inCheck(board, !board->color)) {
            unmakeMove(board, *curMove, &undo);
            ++curMove;
            continue;
        }

        ++searchInfo->nodesCount;
        int score = -quiesceSearch(board, searchInfo, -beta, -alpha, height + 1);

        unmakeMove(board, *curMove, &undo);
        if(score >= beta) {
            return beta;
        }
        if(score > alpha) {
           alpha = score;
        }
        ++curMove;
    }

    if(ABORT) {
        return 0;
    }

    return alpha;
}

U64 perftTest(Board* board, int depth, int height) {
    if(!depth) {
        return 1;
    }


    movegen(board, moves[height]);

    U64 result = 0;
    U16* curMove = moves[height];
    Undo undo;
    while(*curMove) {
        makeMove(board, *curMove, &undo);

        U64 count = 0;
        if(!inCheck(board, !board->color)) {
            count = perftTest(board, depth - 1, height + 1);

            if(!height) {
                char mv[6];
                moveToString(*curMove, mv);
                for(int i = 0; i < height; ++i) {
                    printf(" ");
                }
                printf("%s: %llu\n", mv, count);
            }
        }

        result += count;

        unmakeMove(board, *curMove, &undo);

        ++curMove;
    }

    return result;
}

void perft(Board* board, int depth) {
    for(int i = 1; i <= depth; ++i) {
        clock_t start = clock();
        U64 nodes = perftTest(board, i, 0);
        clock_t end = clock();
        if(!(end - start)) {
            end = start + 1;
        }
        
        printf("Perft %d: %llu; speed: %d; time: %.3fs\n", i, nodes, nodes / (end - start), (end - start) / 1000.);
    }
}

void moveOrdering(Board* board, U16* moves, SearchInfo* searchInfo, int height, int depth) {
    if(depth > MAX_PLY - 1) {
        depth = MAX_PLY - 1;
    }
    U16* ptr = moves;
    U16 hashMove = tt[board->key & ttIndex].move;
    int i;

    for(i = 0; *ptr; ++i) {
        movePrice[height][i] = 0;
        U16 toPiece = pieceType(board->squares[MoveTo(*ptr)]);
        
        if(hashMove == *ptr) {
            movePrice[height][i] = 1000000000;
        } else if(toPiece) {
            U16 fromPiece = pieceType(board->squares[MoveFrom(*ptr)]);
            movePrice[height][i] = mvvLvaScores[fromPiece][toPiece] * 1000000;
        } else if(depth < MAX_PLY && searchInfo->killer[board->color][depth] == *ptr) {
            movePrice[height][i] = 100000;
        } else if(depth >= 2 && depth < MAX_PLY && searchInfo->killer[board->color][depth-2] == *ptr) {
            movePrice[height][i] = 99999;
        } else if(depth < MAX_PLY && searchInfo->secondKiller[board->color][depth] == *ptr) {
            movePrice[height][i] = 99998;
        } else if(depth >= 2 && depth < MAX_PLY && searchInfo->secondKiller[board->color][depth-2] == *ptr) {
            movePrice[height][i] = 99997;
        } else if(!toPiece) {
            movePrice[height][i] = history[board->color][MoveFrom(*ptr)][MoveTo(*ptr)];
        }

        if(MoveType(*ptr) == ENPASSANT_MOVE) {
            movePrice[height][i] = mvvLvaScores[PAWN][PAWN] * 1000000;
        }

        if(toPiece) {
            int seeScore = see(board, MoveTo(*ptr), board->squares[MoveTo(*ptr)], MoveFrom(*ptr), board->squares[MoveFrom(*ptr)]);
            if(seeScore < 0 && hashMove != *ptr) {
                movePrice[height][i] = seeScore;
            }
        }

        
        if(MoveType(*ptr) == PROMOTION_MOVE) {
            if(MovePromotionPiece(*ptr) == QUEEN) {
                movePrice[height][i] = 999999999;
            } else {
                movePrice[height][i] = 0;
            }
        } 
        

        if(searchInfo->bestMove == *ptr && !height) {
            movePrice[height][i] = 1000000000;
        } 

        ++ptr;
    }

    sort(moves, i, height);
}

void sort(U16* moves, int count, int height) {
    int i, j, key;
    U16 keyMove;
    for (i = 1; i < count; i++)  { 
        key = movePrice[height][i];
        keyMove = moves[i];
        j = i - 1; 
    
        while (j >= 0 && movePrice[height][j] < key) { 
            movePrice[height][j + 1] = movePrice[height][j];
            moves[j + 1] = moves[j];
            --j;
        } 
        movePrice[height][j + 1] = key;
        moves[j + 1] = keyMove;
    }
}

void initSearch() {
    for(int attacker = 1; attacker < 7; ++attacker) {
        for(int victim = 1; victim < 7; ++victim) {
            mvvLvaScores[attacker][victim] = 64 * victim - attacker;
        }
    }

    for(int i = 0; i < MAX_PLY; ++i) {
        for(int j = 0; j < 64; ++j) {
            lmr[i][j]  = 0.75 + log(i) * log(j) / 2.25;
        }
    }

    clearHistory();
}

void resetSearchInfo(SearchInfo* info, TimeManager tm) {
    memset(info, 0, sizeof(SearchInfo));
    info->tm = tm;
    setAbort(0);
    compressHistory();
}

void replaceTransposition(Transposition* tr, Transposition new_tr, int height) {
    int score = new_tr.eval;

    if(score > MATE_SCORE - 100) {
        score += height;
    } else if(score < -MATE_SCORE + 100) {
        score -= height;
    }

    if(tr->age + 5 < ttAge || !tr->evalType) {
        replaceTranspositionEntry(tr, &new_tr);
        return;
    }

    if(new_tr.depth >= tr->depth) {
        new_tr.eval = score;
        if(new_tr.evalType == upperbound && tr->evalType != upperbound) {
            return;
        }
        replaceTranspositionEntry(tr, &new_tr);
    }
}

void setAbort(int val) {
    pthread_mutex_lock(&mutex);
    ABORT = val;
    pthread_mutex_unlock(&mutex);
}

void clearHistory() {
    memset(history, 0, 2*64*64 * sizeof(int));
}
void compressHistory() {
    for(int i = 0; i < 64; ++i) {
        for(int j = 0; j < 64; ++j) {
            history[WHITE][i][j] /= 100;
            history[BLACK][i][j] /= 100;
        }   
    }
}

int isKiller(SearchInfo* info, int side, U16 move, int depth) {
    if(info->killer[side][depth] == move || info->secondKiller[side][depth] == move) {
        return 1;
    }

    if(depth >= 2) {
        if(info->killer[side][depth - 2] == move || info->secondKiller[side][depth - 2] == move) {
            return 1;
        }
    }

    return 0;
}