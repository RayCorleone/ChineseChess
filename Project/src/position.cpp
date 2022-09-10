#include "define.h"
#include "pregen.h"
#include "position.h"


// 起始局面的FEN串
const char *const cszStartFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r";

// 棋子类型对应的棋子符号
const char *const numToTy = "KABNRCP";

const int tyToNum[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6
};

// 棋子的简单分值，只在简单比较时作参考
const int basicScore[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  5, 1, 1, 1, 1, 3, 3, 4, 4, 3, 3, 2, 2, 2, 2, 2,
  5, 1, 1, 1, 1, 3, 3, 4, 4, 3, 3, 2, 2, 2, 2, 2,
};

const uint8_t symCorArr[256] = {
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
  0, 0, 0, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0, 0, 0, 0,
  0, 0, 0, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0, 0, 0, 0,
  0, 0, 0, 0x5b, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0, 0, 0, 0,
  0, 0, 0, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0, 0, 0, 0,
  0, 0, 0, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0, 0, 0, 0,
  0, 0, 0, 0x8b, 0x8a, 0x89, 0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0, 0, 0, 0,
  0, 0, 0, 0x9b, 0x9a, 0x99, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0, 0, 0, 0,
  0, 0, 0, 0xab, 0xaa, 0xa9, 0xa8, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0, 0, 0, 0,
  0, 0, 0, 0xbb, 0xba, 0xb9, 0xb8, 0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0, 0, 0, 0,
  0, 0, 0, 0xcb, 0xca, 0xc9, 0xc8, 0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0, 0, 0, 0,
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
  0, 0, 0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0,
};

// FEN串中棋子标识，注意这个函数只能识别大写字母，因此用小写字母时，首先必须转换为大写
int FenPiece(int nArg) {
    switch (nArg) {
    case 'K':
        return 0;
    case 'A':
        return 1;
    case 'B':
    case 'E':
        return 2;
    case 'N':
    case 'H':
        return 3;
    case 'R':
        return 4;
    case 'C':
        return 5;
    case 'P':
        return 6;
    default:
        return 7;
    }
}


// 棋盘上增加棋子
void myPosition::addChess(int sq, int pc, bool bDel) {
    int pt;

    if (bDel) {
        this->ucpcSquares[sq] = 0;
        this->ucsqPieces[pc] = 0;
    }
    else {
        this->ucpcSquares[sq] = pc;
        this->ucsqPieces[pc] = sq;
    }
    this->wBitRanks[RANK_Y(sq)] ^= PreGen.wBitRankMask[sq];
    this->wBitFiles[FILE_X(sq)] ^= PreGen.wBitFileMask[sq];
    this->dwBitPiece ^= bitP(pc);
    pt = pcType(pc);
    if (pc < 32) {
        if (bDel) {
            this->redScore -= PreEval.vlWhitePieces[pt][sq];
        }
        else {
            this->redScore += PreEval.vlWhitePieces[pt][sq];
        }
    }
    else {
        if (bDel) {
            this->blackScore -= PreEval.vlBlackPieces[pt][sq];
        }
        else {
            this->blackScore += PreEval.vlBlackPieces[pt][sq];
        }
        pt += 7;
    }
    this->zobr.Xor(PreGen.zobrTable[pt][sq]);
}

// 移动棋子
int myPosition::moveChess(int mv) {
    int sqSrc, sqDst, pcMoved, pcCaptured, pt;
    uint8_t* lpucvl;

    // 得到移动的棋子序号和被吃的棋子序号；
    sqSrc = getSta(mv);
    sqDst = getDir(mv);
    pcMoved = this->ucpcSquares[sqSrc];
    pcCaptured = this->ucpcSquares[sqDst];
    if (pcCaptured == 0) {

        // 如果没有被吃的棋子，那么更新目标格的位行和位列。
        this->wBitRanks[RANK_Y(sqDst)] ^= PreGen.wBitRankMask[sqDst];
        this->wBitFiles[FILE_X(sqDst)] ^= PreGen.wBitFileMask[sqDst];
    }
    else {
        // 如果有被吃的棋子，那么更新棋子位，从"ucsqPieces"数组中清除被吃棋，同时更新子力价值、位行位列、Zobrist键值和校验锁
        this->ucsqPieces[pcCaptured] = 0;
        this->dwBitPiece ^= bitP(pcCaptured);
        pt = pcType(pcCaptured);
        if (pcCaptured < 32) {
            this->redScore -= PreEval.vlWhitePieces[pt][sqDst];
        }
        else {
            this->blackScore -= PreEval.vlBlackPieces[pt][sqDst];
            pt += 7;
        }
        this->zobr.Xor(PreGen.zobrTable[pt][sqDst]);
    }

    //从"ucpcSquares"和"ucsqPieces"数组中移动棋子，同时更新位行、位列、子力价值、位行位列、Zobrist键值和校验锁
    this->ucpcSquares[sqSrc] = 0;
    this->ucpcSquares[sqDst] = pcMoved;
    this->ucsqPieces[pcMoved] = sqDst;
    this->wBitRanks[RANK_Y(sqSrc)] ^= PreGen.wBitRankMask[sqSrc];
    this->wBitFiles[FILE_X(sqSrc)] ^= PreGen.wBitFileMask[sqSrc];
    pt = pcType(pcMoved);
    if (pcMoved < 32) {
        lpucvl = PreEval.vlWhitePieces[pt];
        this->redScore += lpucvl[sqDst] - lpucvl[sqSrc];
    }
    else {
        lpucvl = PreEval.vlBlackPieces[pt];
        this->blackScore += lpucvl[sqDst] - lpucvl[sqSrc];
        pt += 7;
    }
    this->zobr.Xor(PreGen.zobrTable[pt][sqDst], PreGen.zobrTable[pt][sqSrc]);
    return pcCaptured;
}

// 撤消移动棋子
void myPosition::UndoC(int mv, int pcCaptured) {
    int sqSrc, sqDst, pcMoved;
    sqSrc = getSta(mv);
    sqDst = getDir(mv);
    pcMoved = this->ucpcSquares[sqDst];
    this->ucpcSquares[sqSrc] = pcMoved;
    this->ucsqPieces[pcMoved] = sqSrc;
    this->wBitRanks[RANK_Y(sqSrc)] ^= PreGen.wBitRankMask[sqSrc];
    this->wBitFiles[FILE_X(sqSrc)] ^= PreGen.wBitFileMask[sqSrc];
    if (pcCaptured > 0) {
        this->ucpcSquares[sqDst] = pcCaptured;
        this->ucsqPieces[pcCaptured] = sqDst;
        this->dwBitPiece ^= bitP(pcCaptured);
    }
    else {
        this->ucpcSquares[sqDst] = 0;
        this->wBitRanks[RANK_Y(sqDst)] ^= PreGen.wBitRankMask[sqDst];
        this->wBitFiles[FILE_X(sqDst)] ^= PreGen.wBitFileMask[sqDst];
    }
}

// 过河
int myPosition::passRiver(int sq) {
    int pcCaptured, pcPromoted, pt;


    //之前棋子的序号；
    pcCaptured = this->ucpcSquares[sq];
    pcPromoted = whichSide(this->nowSide) + Bsf(~this->wBitPiece[this->nowSide] & PAWN_BITPIECE);

    //去掉过河前棋子，同时更新子力价值、Zobrist键值和校验锁
    this->dwBitPiece ^= bitP(pcPromoted) ^ bitP(pcCaptured);
    this->ucsqPieces[pcCaptured] = 0;
    pt = pcType(pcCaptured);
    if (pcCaptured < 32) {
        this->redScore -= PreEval.vlWhitePieces[pt][sq];
    }
    else {
        this->blackScore -= PreEval.vlBlackPieces[pt][sq];
        pt += 7;
    }
    this->zobr.Xor(PreGen.zobrTable[pt][sq]);

    //加上过河后棋子，同时更新子力价值、Zobrist键值和校验锁
    this->ucpcSquares[sq] = pcPromoted;
    this->ucsqPieces[pcPromoted] = sq;
    pt = pcType(pcPromoted);
    if (pcPromoted < 32) {
        this->redScore += PreEval.vlWhitePieces[pt][sq];
    }
    else {
        this->blackScore += PreEval.vlBlackPieces[pt][sq];
        pt += 7;
    }
    this->zobr.Xor(PreGen.zobrTable[pt][sq]);
    return pcCaptured;
}

// 回来
void myPosition::backRiver(int sq, int pcCaptured) {
    int pcPromoted;
    pcPromoted = this->ucpcSquares[sq];
    this->ucsqPieces[pcPromoted] = 0;
    this->ucpcSquares[sq] = pcCaptured;
    this->ucsqPieces[pcCaptured] = sq;
    this->dwBitPiece ^= bitP(pcPromoted) ^ bitP(pcCaptured);
}


// 执行一个着法
bool myPosition::doAMove(int mv) {
    int sq, pcCaptured;
    uint32_t dwOldZobristKey;
    myDrawback* lprbs;

    // 达到最大着法数，非法
    if (this->mvsNum == MAX_ROLL) {
        return false;
    }

    // 保存原来的Zobrist键值
    dwOldZobristKey = this->zobr.dwKey;
    stoStus();

    // 移动棋子，记住吃掉的子
    sq = getSta(mv);
    if (sq == getDir(mv)) {
        pcCaptured = passRiver(sq);
    }
    else {
        pcCaptured = moveChess(mv);
        if (getCheckingOne(CHECK_LAZY) > 0) {
            UndoC(mv, pcCaptured);
            drawBack();
            return false;
        }
    }

    changeSides();

    //记录Zobrist键值
    if (this->ucRepHash[dwOldZobristKey & DUL_HT_LEN] == 0) {
        this->ucRepHash[dwOldZobristKey & DUL_HT_LEN] = this->mvsNum;
    }

    // 把着法保存到历史着法列表中，并记住吃掉的子和将军状态
    lprbs = this->rbsList + this->mvsNum;
    lprbs->mvs.bmv = mv;
    lprbs->mvs.ChkChs = getCheckingOne();

    //设置和棋着法数
    if (pcCaptured == 0) {
        if (lprbs->mvs.ChkChs == 0) {
            lprbs->mvs.ChkChs = -chasedBy(mv);
        }
        if (theCheckingM().CptDrw == -100) {
            lprbs->mvs.CptDrw = -100;
        }
        else {
            lprbs->mvs.CptDrw = min((int)theCheckingM().CptDrw, 0) - (lprbs->mvs.ChkChs > 0 || theCheckingM().ChkChs > 0 ? 0 : 1);
        }
    }
    else {
        lprbs->mvs.CptDrw = pcCaptured;
    }
    this->mvsNum++;
    this->nDistance++;

    return true;
}

// 撤消一个着法
void myPosition::unDoAMove(void) {
    int sq;
    myDrawback* lprbs;
    this->mvsNum--;
    this->nDistance--;
    lprbs = this->rbsList + this->mvsNum;
    sq = getSta(lprbs->mvs.bmv);
    if (sq == getDir(lprbs->mvs.bmv)) {
        backRiver(sq, lprbs->mvs.CptDrw);
    }
    else {
        UndoC(lprbs->mvs.bmv, lprbs->mvs.CptDrw);
    }
    this->nowSide = OPP_SIDE(this->nowSide);
    drawBack();
    if (this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] == this->mvsNum) {
        this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] = 0;
    }
}

// 执行一个空着
void myPosition::nullMove(void) {
    if (this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] == 0) {
        this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] = this->mvsNum;
    }
    stoStus();
    changeSides();
    this->rbsList[mvsNum].mvs.dwmv = 0; // bmv, Chk, CptDrw, ChkChs = 0
    this->mvsNum++;
    this->nDistance++;
}

// 撤消一个空着
void myPosition::unDoNullMove(void) {
    this->mvsNum--;
    this->nDistance--;
    this->nowSide = OPP_SIDE(this->nowSide);
    drawBack();
    if (this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] == this->mvsNum) {
        this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] = 0;
    }
}

// FEN串识别
void myPosition::recoByFen(const char* szFen) {
    int i, j, k;
    int pcWhite[7];
    int pcBlack[7];
    const char* lpFen;
    // FEN串的识别包括以下几个步骤：
    //  初始化，清空棋盘
    pcWhite[0] = whichSide(0) + GENERAL_FROM;
    pcWhite[1] = whichSide(0) + OFFIER_FROM;
    pcWhite[2] = whichSide(0) + ELE_FROM;
    pcWhite[3] = whichSide(0) + H_FROM;
    pcWhite[4] = whichSide(0) + JU_FROM;
    pcWhite[5] = whichSide(0) + CANNON_FROM;
    pcWhite[6] = whichSide(0) + PAWN_FROM;
    for (i = 0; i < 7; i++) {
        pcBlack[i] = pcWhite[i] + 16;
    }
    emptyBoard();
    lpFen = szFen;
    if (*lpFen == '\0') {
        emptyDraw();
        return;
    }
    //  读取棋盘上的棋子
    i = RANK_TOP;
    j = FILE_LEFT;
    while (*lpFen != ' ') {
        if (*lpFen == '/') {
            j = FILE_LEFT;
            i++;
            if (i > RANK_BOTTOM) {
                break;
            }
        }
        else if (*lpFen >= '1' && *lpFen <= '9') {
            j += (*lpFen - '0');
        }
        else if (*lpFen >= 'A' && *lpFen <= 'Z') {
            if (j <= FILE_RIGHT) {
                k = FenPiece(*lpFen);
                if (k < 7) {
                    if (pcWhite[k] < 32) {
                        if (this->ucsqPieces[pcWhite[k]] == 0) {
                            addChess(COORD_XY(j, i), pcWhite[k]);
                            pcWhite[k] ++;
                        }
                    }
                }
                j++;
            }
        }
        else if (*lpFen >= 'a' && *lpFen <= 'z') {
            if (j <= FILE_RIGHT) {
                k = FenPiece(*lpFen + 'A' - 'a');
                if (k < 7) {
                    if (pcBlack[k] < 48) {
                        if (this->ucsqPieces[pcBlack[k]] == 0) {
                            addChess(COORD_XY(j, i), pcBlack[k]);
                            pcBlack[k] ++;
                        }
                    }
                }
                j++;
            }
        }
        lpFen++;
        if (*lpFen == '\0') {
            emptyDraw();
            return;
        }
    }
    lpFen++;
    //  确定轮到哪方走
    if (*lpFen == 'b') {
        changeSides();
    }
    //  把局面设成“不可逆”
    emptyDraw();
}

// 生成FEN串
void myPosition::genFen(char* szFen) const {
    int i, j, k, pc;
    char* lpFen;

    lpFen = szFen;
    for (i = RANK_TOP; i <= RANK_BOTTOM; i++) {
        k = 0;
        for (j = FILE_LEFT; j <= FILE_RIGHT; j++) {
            pc = this->ucpcSquares[COORD_XY(j, i)];
            if (pc != 0) {
                if (k > 0) {
                    *lpFen = k + '0';
                    lpFen++;
                    k = 0;
                }
                *lpFen = pcByte(pcType(pc)) + (pc < 32 ? 0 : 'a' - 'A');
                lpFen++;
            }
            else {
                k++;
            }
        }
        if (k > 0) {
            *lpFen = k + '0';
            lpFen++;
        }
        *lpFen = '/';
        lpFen++;
    }
    *(lpFen - 1) = ' '; // 把最后一个'/'替换成' '
    *lpFen = (this->nowSide == 0 ? 'w' : 'b');
    lpFen++;
    *lpFen = '\0';
}

// 局面镜像
void myPosition::symSitu(void) {
    int i, sq, nMoveNumSave;
    uint16_t wmvList[MAX_ROLL];
    uint8_t ucsqList[32];

    nMoveNumSave = this->mvsNum;
    for (i = 1; i < nMoveNumSave; i++) {
        wmvList[i] = this->rbsList[i].mvs.bmv;
    }

    for (i = 1; i < nMoveNumSave; i++) {
        unDoAMove();
    }
    for (i = 16; i < 48; i++) {
        sq = this->ucsqPieces[i];
        ucsqList[i - 16] = sq;
        if (sq != 0) {
            addChess(sq, i, 1);
        }
    }

    for (i = 16; i < 48; i++) {
        sq = ucsqList[i - 16];
        if (sq != 0) {
            addChess(symCo(sq), i);
        }
    }

    emptyDraw();
    for (i = 1; i < nMoveNumSave; i++) {
        doAMove(symM(wmvList[i]));
    }
}


bool myPosition::ifLegalM(int mv) const {
    int sqSrc, sqDst, sqPin, pcMoved, pcCaptured, x, y, nSideTag;
    // 检查要走的子是否存在
    nSideTag = whichSide(this->nowSide);
    sqSrc = getSta(mv);
    sqDst = getDir(mv);
    pcMoved = this->ucpcSquares[sqSrc];
    if ((pcMoved & nSideTag) == 0) {
        return false;
    }

    //检查吃到的子是否为对方棋子(如果有吃子并且没有过河的话)
    pcCaptured = this->ucpcSquares[sqDst];
    if (sqSrc != sqDst && (pcCaptured & nSideTag) != 0) {
        return false;
    }
    switch (pcIn(pcMoved)) {

        //如果是帅(将)或仕(士)，则先看是否在九宫内，再看是否是合理位移
    case GENERAL_FROM:
        return IN_FORT(sqDst) && KING_SPAN(sqSrc, sqDst);
    case OFFIER_FROM:
    case OFFIER_TO:
        if (sqSrc == sqDst) {
            return CAN_PROMOTE(sqSrc) && ifPassed();
        }
        else {
            return IN_FORT(sqDst) && ADVISOR_SPAN(sqSrc, sqDst);
        }

        // 如果是相(象)，则先看是否过河，再看是否是合理位移，最后看有没有被塞象眼
    case ELE_FROM:
    case ELE_TO:
        if (sqSrc == sqDst) {
            return CAN_PROMOTE(sqSrc) && ifPassed();
        }
        else {
            return SAME_HALF(sqSrc, sqDst) && BISHOP_SPAN(sqSrc, sqDst) && this->ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0;
        }

        // 如果是马，则先看看是否是合理位移，再看有没有被蹩马腿
    case H_FROM:
    case H_TO:
        sqPin = KNIGHT_PIN(sqSrc, sqDst);
        return sqPin != sqSrc && this->ucpcSquares[sqPin] == 0;

        // 如果是车，则先看是横向移动还是纵向移动，再读取位行或位列的着法预生成数组
    case JU_FROM:
    case JU_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (x == FILE_X(sqDst)) {
            if (pcCaptured == 0) {
                return (FileMaskPtr(x, y)->wNonCap & PreGen.wBitFileMask[sqDst]) != 0;
            }
            else {
                return (FileMaskPtr(x, y)->wRookCap & PreGen.wBitFileMask[sqDst]) != 0;
            }
        }
        else if (y == RANK_Y(sqDst)) {
            if (pcCaptured == 0) {
                return (RankMaskPtr(x, y)->wNonCap & PreGen.wBitRankMask[sqDst]) != 0;
            }
            else {
                return (RankMaskPtr(x, y)->wRookCap & PreGen.wBitRankMask[sqDst]) != 0;
            }
        }
        else {
            return false;
        }

        //炮和车一样
    case CANNON_FROM:
    case CANNON_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (x == FILE_X(sqDst)) {
            if (pcCaptured == 0) {
                return (FileMaskPtr(x, y)->wNonCap & PreGen.wBitFileMask[sqDst]) != 0;
            }
            else {
                return (FileMaskPtr(x, y)->wCannonCap & PreGen.wBitFileMask[sqDst]) != 0;
            }
        }
        else if (y == RANK_Y(sqDst)) {
            if (pcCaptured == 0) {
                return (RankMaskPtr(x, y)->wNonCap & PreGen.wBitRankMask[sqDst]) != 0;
            }
            else {
                return (RankMaskPtr(x, y)->wCannonCap & PreGen.wBitRankMask[sqDst]) != 0;
            }
        }
        else {
            return false;
        }

        //兵(卒)按红方和黑方分情况讨论
    default:
        if (AWAY_HALF(sqDst, this->nowSide) && (sqDst == sqSrc - 1 || sqDst == sqSrc + 1)) {
            return true;
        }
        else {
            return sqDst == SQUARE_FORWARD(sqSrc, this->nowSide);
        }
    }
}

// 将军检测
int myPosition::getCheckingOne(bool bLazy) const {
    int pcCheckedBy, i, sqSrc, sqDst, sqPin, pc, x, y, nOppSideTag;
    mySlideMask* lpsmsRank, * lpsmsFile;

    pcCheckedBy = 0;
    nOppSideTag = resWhichSide(this->nowSide);

    // 判断帅(将)是否在棋盘上
    sqSrc = this->ucsqPieces[whichSide(this->nowSide)];
    if (sqSrc == 0) {
        return 0;
    }

    //获得帅(将)所在格子的位行和位列
    x = FILE_X(sqSrc);
    y = RANK_Y(sqSrc);
    lpsmsRank = RankMaskPtr(x, y);
    lpsmsFile = FileMaskPtr(x, y);

    // 判断是否将帅对脸
    sqDst = this->ucsqPieces[nOppSideTag + GENERAL_FROM];
    if (sqDst != 0) {
        if (x == FILE_X(sqDst) && (lpsmsFile->wRookCap & PreGen.wBitFileMask[sqDst]) != 0) {
            return CHECK_MULTI;
        }
    }

    //  判断是否被马将军
    for (i = H_FROM; i <= H_TO; i++) {
        sqDst = this->ucsqPieces[nOppSideTag + i];
        if (sqDst != 0) {
            sqPin = KNIGHT_PIN(sqDst, sqSrc); // 注意，sqSrc和sqDst是反的！
            if (sqPin != sqDst && this->ucpcSquares[sqPin] == 0) {
                if (bLazy || pcCheckedBy > 0) {
                    return CHECK_MULTI;
                }
                pcCheckedBy = nOppSideTag + i;
            }
        }
    }

    // 判断是否被车将军或将帅对脸
    for (i = JU_FROM; i <= JU_TO; i++) {
        sqDst = this->ucsqPieces[nOppSideTag + i];
        if (sqDst != 0) {
            if (x == FILE_X(sqDst)) {
                if ((lpsmsFile->wRookCap & PreGen.wBitFileMask[sqDst]) != 0) {
                    if (bLazy || pcCheckedBy > 0) {
                        return CHECK_MULTI;
                    }
                    pcCheckedBy = nOppSideTag + i;
                }
            }
            else if (y == RANK_Y(sqDst)) {
                if ((lpsmsRank->wRookCap & PreGen.wBitRankMask[sqDst]) != 0) {
                    if (bLazy || pcCheckedBy > 0) {
                        return CHECK_MULTI;
                    }
                    pcCheckedBy = nOppSideTag + i;
                }
            }
        }
    }

    // 判断是否被炮将军
    for (i = CANNON_FROM; i <= CANNON_TO; i++) {
        sqDst = this->ucsqPieces[nOppSideTag + i];
        if (sqDst != 0) {
            if (x == FILE_X(sqDst)) {
                if ((lpsmsFile->wCannonCap & PreGen.wBitFileMask[sqDst]) != 0) {
                    if (bLazy || pcCheckedBy > 0) {
                        return CHECK_MULTI;
                    }
                    pcCheckedBy = nOppSideTag + i;
                }
            }
            else if (y == RANK_Y(sqDst)) {
                if ((lpsmsRank->wCannonCap & PreGen.wBitRankMask[sqDst]) != 0) {
                    if (bLazy || pcCheckedBy > 0) {
                        return CHECK_MULTI;
                    }
                    pcCheckedBy = nOppSideTag + i;
                }
            }
        }
    }

    //判断是否被兵(卒)将军
    for (sqDst = sqSrc - 1; sqDst <= sqSrc + 1; sqDst += 2) {

        pc = this->ucpcSquares[sqDst];
        if ((pc & nOppSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
            if (bLazy || pcCheckedBy > 0) {
                return CHECK_MULTI;
            }
            pcCheckedBy = nOppSideTag + i;
        }
    }
    pc = this->ucpcSquares[SQUARE_FORWARD(sqSrc, this->nowSide)];
    if ((pc & nOppSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
        if (bLazy || pcCheckedBy > 0) {
            return CHECK_MULTI;
        }
        pcCheckedBy = nOppSideTag + i;
    }
    return pcCheckedBy;
}

// 判断是否被将死
bool myPosition::ifLose(void) {
    int i, nGenNum;
    myMove mvsGen[MAX_MOVS];
    nGenNum = genEatMvs(mvsGen);
    for (i = 0; i < nGenNum; i++) {
        if (doAMove(mvsGen[i].bmv)) {
            unDoAMove();
            return false;
        }
    }
    nGenNum = genMvsWithoutEat(mvsGen);
    for (i = 0; i < nGenNum; i++) {
        if (doAMove(mvsGen[i].bmv)) {
            unDoAMove();
            return false;
        }
    }
    return true;
}

// 设置将军状态位
inline void SetPerpCheck(uint32_t& dwPerpCheck, int nChkChs) {
    if (nChkChs == 0) {
        dwPerpCheck = 0;
    }
    else if (nChkChs > 0) {
        dwPerpCheck &= 0x10000;
    }
    else {
        dwPerpCheck &= (1 << -nChkChs);
    }
}

// 重复局面检测
int myPosition::sameSitu(int nRecur) const {

    int sd;
    uint32_t dwPerpCheck, dwOppPerpCheck;
    const myDrawback* lprbs;

    if (this->ucRepHash[this->zobr.dwKey & DUL_HT_LEN] == 0) {
        return REP_NONE;
    }

    // 2. 初始化
    sd = OPP_SIDE(this->nowSide);
    dwPerpCheck = dwOppPerpCheck = 0x1ffff;
    lprbs = this->rbsList + this->mvsNum - 1;

    //  检查上一个着法，如果是空着或吃子着法，就不可能有重复了
    while (lprbs->mvs.bmv != 0 && lprbs->mvs.CptDrw <= 0) {

        // 判断双方的长打级别，0表示无长打，0xffff表示长捉，0x10000表示长将
        if (sd == this->nowSide) {
            SetPerpCheck(dwPerpCheck, lprbs->mvs.ChkChs);

            //  寻找重复局面，如果重复次数达到预定次数，则返回重复记号
            if (lprbs->zobr.dwLock0 == this->zobr.dwLock0 && lprbs->zobr.dwLock1 == this->zobr.dwLock1) {
                nRecur--;
                if (nRecur == 0) {
                    dwPerpCheck = ((dwPerpCheck & 0xffff) == 0 ? dwPerpCheck : 0xffff);
                    dwOppPerpCheck = ((dwOppPerpCheck & 0xffff) == 0 ? dwOppPerpCheck : 0xffff);
                    return dwPerpCheck > dwOppPerpCheck ? REP_LOSS : dwPerpCheck < dwOppPerpCheck ? REP_WIN : REP_DRAW;
                }
            }

        }
        else {
            SetPerpCheck(dwOppPerpCheck, lprbs->mvs.ChkChs);
        }

        sd = OPP_SIDE(sd);
        lprbs--;
    }
    return REP_NONE;
}