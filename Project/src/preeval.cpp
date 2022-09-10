#include "define.h"
#include "pregen.h"
#include "position.h"
#include "preeval.h"
#include"my_evaluate.h"

void myPosition::PreEvaluate(void) {
    outer_PreEvaluate(ucsqPieces, dwBitPiece, redScore, blackScore);
}