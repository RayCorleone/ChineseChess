#include <iostream>
#include <string.h>
#include "define.h"
#include "ucci.h"

// UCCI指令分析模块由三各UCCI指令解释器组成。
// 其中第一个解释器"BootLine()"最简单，只用来接收引擎启动后的第一行指令
// 输入"ucci"时就返回"UC_UCCI"，否则一律返回"UC_DEFULT"
// 前两个解释器都等待是否有输入，如果没有输入则执行待机指令"SLEEP()" -> 在 base.h 中
// 而第三个解释器"BusyLine()"，只用在引擎思考时，则在没有输入时直接返回"UC_DEFULT"

const int MAX_ROLL = 1024;
const int LINE_INPUT_MAX_CHAR = 8192;

static char szFen[LINE_INPUT_MAX_CHAR];     //fen串
static unsigned int dwCoordList[MAX_ROLL];  //后续着法表

// 判断是否有走法串
static bool IsMoves(char*& sz1, const char* sz2) {
    char* lpsz;
    lpsz = strstr(sz1, sz2);
    if (lpsz == NULL)
        return false;
    else {
        sz1 = lpsz + strlen(sz2);
        return true;
    }
}

// 对position 指令加工处理
static bool ParsePos(UCStruct& Ucci, char* s) {
    int i;
    int end;

    if (_strnicmp(s, "fen ", strlen("fen ")) == 0) { // 判断FEN串
        s += strlen("fen ");
        end = strcspn(s, "-") - 1;
        strncpy_s(szFen, s, end);
        Ucci.szFenStr = szFen;
    }
    else if (_strnicmp(s, "startpos", strlen("startpos")) == 0) {   // 判断startpos
        Ucci.szFenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r";
    }
    else
        return false;
    //·输出fen串：std::cout << "·Fen = " << Ucci.szFenStr << ";" << std::endl;

    // 寻找是否有"moves"关键字
    Ucci.mvsNum = 0;
    if (IsMoves(s, " moves ")) {
        *(s - strlen(" moves ")) = '\0';
        Ucci.mvsNum = (int)(strlen(s) + 1) / 5; // 提示："moves"后面的每个着法都是1个空格和4个字符
        //·输出nMove：std::cout << "·MoveNum = " << Ucci.mvsNum << std::endl;
        for (i = 0; i < Ucci.mvsNum; i++) {
            dwCoordList[i] = *(unsigned int*)s; // 4个字符可转换为一个"unsigned int"，存储和处理起来方便
            s += sizeof(unsigned int) + 1;
        }
        Ucci.lpdwMovesCoord = dwCoordList;

        //·输出moves：
        //for (i = 0; i < Ucci.mvsNum; i++)
        //    std::cout << "·Moves " << i + 1 << Ucci.lpdwMovesCoord[i] << std::endl;
    }
    return true;
}

// UCCI引擎启动的第一条指令
UcciCommand BootLine() {
    char szStr[LINE_INPUT_MAX_CHAR];

    while (!std::cin.getline(szStr, LINE_INPUT_MAX_CHAR))
        SLEEP();
    if (strcmp(szStr, "ucci") == 0) // 1."ucci"指令
        return UC_UCCI;
    else
        return UC_DEFULT;
}

// 引擎空闲时接收指令
UcciCommand IdleLine(UCStruct& Ucci) {
    char szStr[LINE_INPUT_MAX_CHAR];
    char* lp;

    while (!std::cin.getline(szStr, LINE_INPUT_MAX_CHAR))
        SLEEP();
    lp = szStr;

    if (false) {}
    // 2."isready"指令
    else if (strcmp(lp, "isready") == 0)
        return UC_READY;
    // 3."position"指令
    else if (_strnicmp(lp, "position ", strlen("position ")) == 0) {
        lp += strlen("position ");
        return ParsePos(Ucci, lp) ? UC_POS : UC_DEFULT;
    }
    // 4."go"指令
    else if (_strnicmp(lp, "go ", strlen("go ")) == 0) {
        Ucci.nTime = UCCI_TIME_DEFULT; // 为 nTime 设定默认值
        lp += strlen("go ");
        if (_strnicmp(lp, "time ", strlen("time ")) == 0) {   // 有"time"选项, 更新 nTime 值
            lp += strlen("time ");
            sscanf_s(lp, "%d", &Ucci.nTime);
        }
        //·输出时间：std::cout << "·Time = " << Ucci.nTime << std::endl;
        return UC_GO;
    }
    else if (strcmp(lp, "quit") == 0)       // 5."quit"指令
        return UC_QUIT;
    else
        return UC_DEFULT;
}

// 引擎思考时接收指令
UcciCommand BusyLine(UCStruct& Ucci) {
    char szStr[LINE_INPUT_MAX_CHAR];

    if (std::cin.getline(szStr, LINE_INPUT_MAX_CHAR)) {
        if (false) {}
        else if (strcmp(szStr, "isready") == 0) // 6."isready"指令
            return UC_READY;
        else if (strcmp(szStr, "quit") == 0)    // 7."quit"指令
            return UC_QUIT;
        else
            return UC_DEFULT;
    }
    else
        return UC_DEFULT;
}