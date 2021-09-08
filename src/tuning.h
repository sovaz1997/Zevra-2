#include "board.h"

typedef struct TuningPosition TuningPosition;
typedef struct LinearEvalPosition LinearEvalPosition;
typedef struct LinearEvalParam LinearEvalParam;

double fun(Board* board);
void makeTuning(Board* board);
int* getValues();
void setValues(int* values, int stage);
double getLinearEval(int positionNumber);
void changeParam(int n, int value);
void printParams();
void printPST(char* name, int* pst, int* curIndex, FILE* f);
void printArray(char* name, int* arr, int* curIndex, int length, FILE* f);
int* transferPST(int* from, int* to, int* curIndex);
int* transfer(int* from, int* to, int* curIndex, int length);
void loadPositions(Board* board);
char** str_split(char* a_str, const char a_delim);
int* calculateLinear(Board* board, int positionNumber);