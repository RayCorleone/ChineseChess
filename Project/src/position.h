#include <string.h>
#include "define.h"
#include "pregen.h"

#ifndef POSITION_H
#define POSITION_H

const int MAX_ROLL = 1024;  
const int MAX_MOVS = 128;       //最多120个着法
const int DRAW_MOVES = 100;     //和棋着法数
const int DUL_HT_LEN = 4095; 

const int BEST_SCORE = 10000;          
const int LONG_LOSE_SCORE = BEST_SCORE - 100; // 长将判负分值
const int KILL_SCORE = BEST_SCORE - 200; // 杀招
const int NULLOKAY_LIM = 200;        
const int NULLSAFE_LIM = 400;        
const int PIEACE_SCORE = 20;            

const bool CHECK_LAZY = true;   
const int CHECK_MULTI = 48;     

// 子力类型编号
const int GENERAL_TY = 0;
const int OFFIER_TY = 1;
const int ELE_TY = 2;
const int H_TY = 3;
const int JU_TYPE = 4;
const int CANNON_TYPE = 5;
const int PAWN_TYPE = 6;

// 开始/结束序号
const int GENERAL_FROM = 0;
const int OFFIER_FROM = 1;
const int OFFIER_TO = 2;
const int ELE_FROM = 3;
const int ELE_TO = 4;
const int H_FROM = 5;
const int H_TO = 6;
const int JU_FROM = 7;
const int JU_TO = 8;
const int CANNON_FROM = 9;
const int CANNON_TO = 10;
const int PAWN_FROM = 11;
const int PAWN_TO = 15;

// 子力的屏蔽位
const int GENERAL_BITPIECE = 1 << GENERAL_FROM;
const int OFFIER_BITPIECE = (1 << OFFIER_FROM) | (1 << OFFIER_TO);
const int ELE_BITPIECE = (1 << ELE_FROM) | (1 << ELE_TO);
const int H_BITPIECE = (1 << H_FROM) | (1 << H_TO);
const int JU_BITPIECE = (1 << JU_FROM) | (1 << JU_TO);
const int CANNON_BITPIECE = (1 << CANNON_FROM) | (1 << CANNON_TO);
const int PAWN_BITPIECE = (1 << PAWN_FROM) | (1 << (PAWN_FROM + 1)) |
    (1 << (PAWN_FROM + 2)) | (1 << (PAWN_FROM + 3)) | (1 << PAWN_TO);
const int ATTACK_BITPIECE = H_BITPIECE | JU_BITPIECE | CANNON_BITPIECE | PAWN_BITPIECE;

inline uint32_t bitP(int pc) {
    return 1 << (pc - 16);
}

inline uint32_t twobitp(int nBitPiece) {
    return nBitPiece + (nBitPiece << 16);
}

const int REP_NONE = 0;
const int REP_DRAW = 1;
const int REP_LOSS = 3;
const int REP_WIN = 5;

inline int whichSide(int sd) {
    return 16 + (sd << 4);
}

inline int resWhichSide(int sd) {
    return 32 - (sd << 4);
}

inline int scoreOfSide(int sd, int score) {
    return (sd == 0 ? score : -score);
}

inline int pcIn(int pc) {
    return pc & 15;
}

extern const char *const cszStartFen;     // 起始局面的FEN串
extern const char *const numToTy;   // 棋子类型对应的棋子符号
extern const int tyToNum[48];        // 棋子序号对应的棋子类型
extern const int basicScore[48];      // 棋子的简单分值
extern const uint8_t symCorArr[256]; // 坐标的镜像(左右对称)数组

inline char pcByte(int pt) {
    return numToTy[pt];
}

inline int pcType(int pc) {
    return tyToNum[pc];
}

inline int baScore(int pc) {
    return basicScore[pc];
}

inline uint8_t symCo(int sq) {
    return symCorArr[sq];
}

// FEN串中棋子标识
int FenPiece(int Arg);

// 复杂着法结构
union myMove {
    uint32_t dwmv;           
    struct {
        uint16_t bmv, wsc;     // 着法和分值
    };
    struct {
        uint8_t Src, Dst;      // 起始和目标
        int8_t CptDrw, ChkChs; 
    };
}; // mvs

// 着法结构
// 返回着法起点
inline int getSta(int mv) { 
    return mv & 255;
}
// 返回着法终点
inline int getDir(int mv) { 
    return mv >> 8;
}
// 得到着法
inline int getMbySD(int sqSrc, int sqDst) {   
    return sqSrc + (sqDst << 8);
}
// 着法转字符串
inline uint32_t mvToStr(int mv) {      
    union {
        char c[4];
        uint32_t dw;
    } Ret;
    Ret.c[0] = FILE_X(getSta(mv)) - FILE_LEFT + 'a';
    Ret.c[1] = '9' - RANK_Y(getSta(mv)) + RANK_TOP;
    Ret.c[2] = FILE_X(getDir(mv)) - FILE_LEFT + 'a';
    Ret.c[3] = '9' - RANK_Y(getDir(mv)) + RANK_TOP;
    return Ret.dw;
}

inline int strToMv(uint32_t dwMoveStr) { // 把字符串转换成着法
    int sqSrc, sqDst;
    char* lpArgPtr;
    lpArgPtr = (char*)&dwMoveStr;
    sqSrc = COORD_XY(lpArgPtr[0] - 'a' + FILE_LEFT, '9' - lpArgPtr[1] + RANK_TOP);
    sqDst = COORD_XY(lpArgPtr[2] - 'a' + FILE_LEFT, '9' - lpArgPtr[3] + RANK_TOP);
    return (IN_BOARD(sqSrc) && IN_BOARD(sqDst) ? getMbySD(sqSrc, sqDst) : 0);
}

inline int symM(int mv) {          // 对着法做镜像
    return getMbySD(symCo(getSta(mv)), symCo(getDir(mv)));
}

// 回滚结构
struct myDrawback {
    myZobrist zobr;   // Zobrist
    int redScore, blackScore; // 红方和黑方的子力价值
    myMove mvs;       // 着法
}; // rbs


// 局面结构
struct myPosition {
    // 基本成员
    int nowSide;             // 0红1黑
    uint8_t ucpcSquares[256]; // 每个格子放的棋子，0表示没有棋子
    uint8_t ucsqPieces[48];   // 每个棋子放的位置，0表示被吃
    myZobrist zobr;       // Zobrist

    // 位结构成员，用来增强棋盘的处理
    union {
        uint32_t dwBitPiece;    // 32位的棋子位，0到31位依次表示序号为16到47的棋子是否还在棋盘上
        uint16_t wBitPiece[2];  // 拆分成两个
    };
    uint16_t wBitRanks[16];   // 位行数组
    uint16_t wBitFiles[16];   // 位列数组

    // 局面评价数据
    int redScore, blackScore;   // 红方和黑方的子力价值

    // 回滚着法，用来检测循环局面
    int mvsNum, nDistance;              // 回滚着法数和搜索深度
    myDrawback rbsList[MAX_ROLL]; // 回滚列表
    uint8_t ucRepHash[DUL_HT_LEN + 1]; // 判断重复局面的迷你置换表

    // 获取着法预生成信息
    mySlideMove* RankMovePtr(int x, int y) const {
        return PreGen.smvRankMoveTab[x - FILE_LEFT] + wBitRanks[y];
    }
    mySlideMove* FileMovePtr(int x, int y) const {
        return PreGen.smvFileMoveTab[y - RANK_TOP] + wBitFiles[x];
    }
    mySlideMask* RankMaskPtr(int x, int y) const {
        return PreGen.smsRankMaskTab[x - FILE_LEFT] + wBitRanks[y];
    }
    mySlideMask* FileMaskPtr(int x, int y) const {
        return PreGen.smsFileMaskTab[y - RANK_TOP] + wBitFiles[x];
    }

    // 棋盘处理过程
    void emptyBoard(void) { // 棋盘初始化
        nowSide = 0;
        memset(ucpcSquares, 0, 256);
        memset(ucsqPieces, 0, 48);
        zobr.InitZero();
        dwBitPiece = 0;
        memset(wBitRanks, 0, 16 * sizeof(uint16_t));
        memset(wBitFiles, 0, 16 * sizeof(uint16_t));
        redScore = blackScore = 0;
    }
    // 交换turn
    void changeSides(void) {
        nowSide = OPP_SIDE(nowSide);
        zobr.Xor(PreGen.zobrPlayer);
    }
    // 保存状态
    void stoStus(void) {
        myDrawback* lprbs;
        lprbs = rbsList + mvsNum;
        lprbs->zobr = zobr;
        lprbs->redScore = redScore;
        lprbs->blackScore = blackScore;
    }
    // 回滚
    void drawBack(void) {   
        myDrawback* lprbs;
        lprbs = rbsList + mvsNum;
        zobr = lprbs->zobr;
        redScore = lprbs->redScore;
        blackScore = lprbs->blackScore;
    }
    void addChess(int sq, int pc, bool ifD = false); 
    int moveChess(int mv);                           
    void UndoC(int mv, int pcCaptured);       
    int passRiver(int sq);                              
    void backRiver(int sq, int pcCaptured);         

    bool doAMove(int mv);   // 执行着法
    void unDoAMove(void); // 撤消着法
    void nullMove(void);     // 空着
    void unDoNullMove(void); // 撤消空着
    void emptyDraw(void) {   
        rbsList[0].mvs.dwmv = 0; // bmv, Chk, CptDrw, ChkChs = 0
        rbsList[0].mvs.ChkChs = getCheckingOne();
        mvsNum = 1;
        nDistance = 0;
        memset(ucRepHash, 0, DUL_HT_LEN + 1);
    }

    // 局面处理
    void recoByFen(const char* szFen); 
    void genFen(char* szFen) const;   
    void symSitu(void);              

    // 着法检测
    bool wellEat(int mv) const {     
        int pcMoved, pcCaptured;
        pcCaptured = ucpcSquares[getDir(mv)];
        if (pcCaptured == 0) return false;
        if (!ifProtected(OPP_SIDE(nowSide), getDir(mv))) return true;
        pcMoved = ucpcSquares[getSta(mv)];
        return baScore(pcCaptured) > baScore(pcMoved);
    }
    bool ifLegalM(int mv) const;            
    int getCheckingOne(bool bLazy = false) const; 
    bool ifLose(void);                       // 已被将死?
    myMove theCheckingM(void) const {        // 将军着法
        return rbsList[mvsNum - 1].mvs;
    }
    //过河
    bool ifPassed(void) const {          
        return (wBitPiece[nowSide] & PAWN_BITPIECE) != PAWN_BITPIECE && theCheckingM().ChkChs <= 0;
    }
    //空着裁剪
    bool ifAblNul(void) const {              
        return (nowSide == 0 ? redScore : blackScore) > NULLOKAY_LIM;
    }
    // 空着裁剪不检验
    bool nullCanPass(void) const {             
        return (nowSide == 0 ? redScore : blackScore) > NULLSAFE_LIM;
    }
    // 和棋?
    bool ifDraw(void) const {               
        return (!PreEval.bPromotion && (dwBitPiece & twobitp(ATTACK_BITPIECE)) == 0) ||
            -theCheckingM().CptDrw >= DRAW_MOVES || mvsNum == MAX_ROLL;
    }
    //重复局面
    int sameSitu(int nRecur = 1) const;   
    // 和棋得分
    int drawScore(void) const {              
        return (nDistance & 1) == 0 ? -PIEACE_SCORE : PIEACE_SCORE;
    }
    // 重复局面得分
    int sameSituScore(int vlRep) const {         
        return vlRep == REP_LOSS ? nDistance - LONG_LOSE_SCORE : vlRep == REP_WIN ? LONG_LOSE_SCORE - nDistance : drawScore();
    }
    // 子力平衡
    int Material(void) const {               
        return scoreOfSide(nowSide, redScore - blackScore) + PreEval.vlAdvanced;
    }
    // 着法生成方法
    //是否被保护
    bool ifProtected(int sd, int sqSrc, int sqExcept = 0) const; 
    //
    int chasedBy(int mv) const;            
    // 计算MVV(LVA)值
    int MvvLva(int sqDst, int pcCaptured, int nLva) const;    
    // 吃子着法
    int genEatMvs(myMove* lpmvs) const;       
    // 不吃子着
    int genMvsWithoutEat(myMove* lpmvs) const;  
    // 全部着法
    int genAllMvs(myMove* lpmvs) const {                 
        int nCapNum;
        nCapNum = genEatMvs(lpmvs);
        return nCapNum + genMvsWithoutEat(lpmvs + nCapNum);
    }

    void PreEvaluate(void);
    int AdvisorShape(void) const;
    int StringHold(void) const;
    int RookMobility(void) const;
    int KnightTrap(void) const;
    int Evaluate(int vlAlpha, int vlBeta) const;
}; // pos

#endif