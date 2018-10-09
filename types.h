#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

//Цвет
enum {
    WHITE = 0,
    BLACK = 1
};

//Названия фигур
enum {
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6
};

//Рокировка
enum Castling {
    WS_CASTLING = 1,
    WL_CASTLING = 2,
    BS_CASTLING = 4,
    BL_CASTLING = 8
};

//Направление
enum {
    UP = 0,
    DOWN = 1
};

//Обозначения фигур
extern const char* PieceName[2];

//Переименование типов
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;

//Структуры
typedef struct Board Board;

#endif