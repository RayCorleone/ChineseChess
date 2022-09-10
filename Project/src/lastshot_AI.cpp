// The Last Shot, The Best Shot
// Fighting !!!!!!!!!!!!

#include <stdio.h>
#include "define.h"
#include "ucci.h"
#include "pregen.h"
#include "position.h"
#include "hash.h"
#include "search.h"

inline void PrintLn(const char* sz) {
    printf("%s\n", sz);
    fflush(stdout);
}

int main(void) {
    UCStruct UcciComm;

    if (BootLine() != UC_UCCI) {
        return 0;
    }
    PreGenInit();
    Search.pos.recoByFen(cszStartFen);
    genHT(26); // 24=16MB, 25=32MB, 26=64MB, ...
    Search.randMNum = 0;
    Search.pos.nDistance = 0;
    Search.pos.PreEvaluate();
    Search.ifQui = false;
    Search.ifHT = true;
    Search.ifBook = true;
    Search.ifNM = true;
    Search.ifKnow = true;
    Search.randomNum.InitRand();
    PrintLn("ucciok");

    // 以下是接收指令和提供对策的循环体
    while (!Search.ifQui) {
        switch (IdleLine(UcciComm)) {
        case UC_READY:
            PrintLn("readyok");
            break;
        case UC_POS:
            BuildPos(Search.pos, UcciComm);
            Search.pos.nDistance = 0;
            Search.pos.PreEvaluate();
            break;
        case UC_GO:
            Search.maxTime = (int)(UcciComm.nTime * 0.96);
            SearchMain(UCCI_MAX_DEPTH);
            break;
        case UC_QUIT:
            Search.ifQui = true;
            break;
        default:
            break;
        }
    }
    delTab();
    PrintLn("bye");
    return 0;
}
