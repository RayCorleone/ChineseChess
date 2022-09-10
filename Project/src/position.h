#include <string.h>
#include "define.h"
#include "pregen.h"

#ifndef POSITION_H
#define POSITION_H

const int MAX_ROLL = 1024;  
const int MAX_MOVS = 128;       //���120���ŷ�
const int DRAW_MOVES = 100;     //�����ŷ���
const int DUL_HT_LEN = 4095; 

const int BEST_SCORE = 10000;          
const int LONG_LOSE_SCORE = BEST_SCORE - 100; // �����и���ֵ
const int KILL_SCORE = BEST_SCORE - 200; // ɱ��
const int NULLOKAY_LIM = 200;        
const int NULLSAFE_LIM = 400;        
const int PIEACE_SCORE = 20;            

const bool CHECK_LAZY = true;   
const int CHECK_MULTI = 48;     

// �������ͱ��
const int GENERAL_TY = 0;
const int OFFIER_TY = 1;
const int ELE_TY = 2;
const int H_TY = 3;
const int JU_TYPE = 4;
const int CANNON_TYPE = 5;
const int PAWN_TYPE = 6;

// ��ʼ/�������
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

// ����������λ
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

extern const char *const cszStartFen;     // ��ʼ�����FEN��
extern const char *const numToTy;   // �������Ͷ�Ӧ�����ӷ���
extern const int tyToNum[48];        // ������Ŷ�Ӧ����������
extern const int basicScore[48];      // ���ӵļ򵥷�ֵ
extern const uint8_t symCorArr[256]; // ����ľ���(���ҶԳ�)����

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

// FEN�������ӱ�ʶ
int FenPiece(int Arg);

// �����ŷ��ṹ
union myMove {
    uint32_t dwmv;           
    struct {
        uint16_t bmv, wsc;     // �ŷ��ͷ�ֵ
    };
    struct {
        uint8_t Src, Dst;      // ��ʼ��Ŀ��
        int8_t CptDrw, ChkChs; 
    };
}; // mvs

// �ŷ��ṹ
// �����ŷ����
inline int getSta(int mv) { 
    return mv & 255;
}
// �����ŷ��յ�
inline int getDir(int mv) { 
    return mv >> 8;
}
// �õ��ŷ�
inline int getMbySD(int sqSrc, int sqDst) {   
    return sqSrc + (sqDst << 8);
}
// �ŷ�ת�ַ���
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

inline int strToMv(uint32_t dwMoveStr) { // ���ַ���ת�����ŷ�
    int sqSrc, sqDst;
    char* lpArgPtr;
    lpArgPtr = (char*)&dwMoveStr;
    sqSrc = COORD_XY(lpArgPtr[0] - 'a' + FILE_LEFT, '9' - lpArgPtr[1] + RANK_TOP);
    sqDst = COORD_XY(lpArgPtr[2] - 'a' + FILE_LEFT, '9' - lpArgPtr[3] + RANK_TOP);
    return (IN_BOARD(sqSrc) && IN_BOARD(sqDst) ? getMbySD(sqSrc, sqDst) : 0);
}

inline int symM(int mv) {          // ���ŷ�������
    return getMbySD(symCo(getSta(mv)), symCo(getDir(mv)));
}

// �ع��ṹ
struct myDrawback {
    myZobrist zobr;   // Zobrist
    int redScore, blackScore; // �췽�ͺڷ���������ֵ
    myMove mvs;       // �ŷ�
}; // rbs


// ����ṹ
struct myPosition {
    // ������Ա
    int nowSide;             // 0��1��
    uint8_t ucpcSquares[256]; // ÿ�����ӷŵ����ӣ�0��ʾû������
    uint8_t ucsqPieces[48];   // ÿ�����ӷŵ�λ�ã�0��ʾ����
    myZobrist zobr;       // Zobrist

    // λ�ṹ��Ա��������ǿ���̵Ĵ���
    union {
        uint32_t dwBitPiece;    // 32λ������λ��0��31λ���α�ʾ���Ϊ16��47�������Ƿ���������
        uint16_t wBitPiece[2];  // ��ֳ�����
    };
    uint16_t wBitRanks[16];   // λ������
    uint16_t wBitFiles[16];   // λ������

    // ������������
    int redScore, blackScore;   // �췽�ͺڷ���������ֵ

    // �ع��ŷ����������ѭ������
    int mvsNum, nDistance;              // �ع��ŷ������������
    myDrawback rbsList[MAX_ROLL]; // �ع��б�
    uint8_t ucRepHash[DUL_HT_LEN + 1]; // �ж��ظ�����������û���

    // ��ȡ�ŷ�Ԥ������Ϣ
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

    // ���̴������
    void emptyBoard(void) { // ���̳�ʼ��
        nowSide = 0;
        memset(ucpcSquares, 0, 256);
        memset(ucsqPieces, 0, 48);
        zobr.InitZero();
        dwBitPiece = 0;
        memset(wBitRanks, 0, 16 * sizeof(uint16_t));
        memset(wBitFiles, 0, 16 * sizeof(uint16_t));
        redScore = blackScore = 0;
    }
    // ����turn
    void changeSides(void) {
        nowSide = OPP_SIDE(nowSide);
        zobr.Xor(PreGen.zobrPlayer);
    }
    // ����״̬
    void stoStus(void) {
        myDrawback* lprbs;
        lprbs = rbsList + mvsNum;
        lprbs->zobr = zobr;
        lprbs->redScore = redScore;
        lprbs->blackScore = blackScore;
    }
    // �ع�
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

    bool doAMove(int mv);   // ִ���ŷ�
    void unDoAMove(void); // �����ŷ�
    void nullMove(void);     // ����
    void unDoNullMove(void); // ��������
    void emptyDraw(void) {   
        rbsList[0].mvs.dwmv = 0; // bmv, Chk, CptDrw, ChkChs = 0
        rbsList[0].mvs.ChkChs = getCheckingOne();
        mvsNum = 1;
        nDistance = 0;
        memset(ucRepHash, 0, DUL_HT_LEN + 1);
    }

    // ���洦��
    void recoByFen(const char* szFen); 
    void genFen(char* szFen) const;   
    void symSitu(void);              

    // �ŷ����
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
    bool ifLose(void);                       // �ѱ�����?
    myMove theCheckingM(void) const {        // �����ŷ�
        return rbsList[mvsNum - 1].mvs;
    }
    //����
    bool ifPassed(void) const {          
        return (wBitPiece[nowSide] & PAWN_BITPIECE) != PAWN_BITPIECE && theCheckingM().ChkChs <= 0;
    }
    //���Ųü�
    bool ifAblNul(void) const {              
        return (nowSide == 0 ? redScore : blackScore) > NULLOKAY_LIM;
    }
    // ���Ųü�������
    bool nullCanPass(void) const {             
        return (nowSide == 0 ? redScore : blackScore) > NULLSAFE_LIM;
    }
    // ����?
    bool ifDraw(void) const {               
        return (!PreEval.bPromotion && (dwBitPiece & twobitp(ATTACK_BITPIECE)) == 0) ||
            -theCheckingM().CptDrw >= DRAW_MOVES || mvsNum == MAX_ROLL;
    }
    //�ظ�����
    int sameSitu(int nRecur = 1) const;   
    // ����÷�
    int drawScore(void) const {              
        return (nDistance & 1) == 0 ? -PIEACE_SCORE : PIEACE_SCORE;
    }
    // �ظ�����÷�
    int sameSituScore(int vlRep) const {         
        return vlRep == REP_LOSS ? nDistance - LONG_LOSE_SCORE : vlRep == REP_WIN ? LONG_LOSE_SCORE - nDistance : drawScore();
    }
    // ����ƽ��
    int Material(void) const {               
        return scoreOfSide(nowSide, redScore - blackScore) + PreEval.vlAdvanced;
    }
    // �ŷ����ɷ���
    //�Ƿ񱻱���
    bool ifProtected(int sd, int sqSrc, int sqExcept = 0) const; 
    //
    int chasedBy(int mv) const;            
    // ����MVV(LVA)ֵ
    int MvvLva(int sqDst, int pcCaptured, int nLva) const;    
    // �����ŷ�
    int genEatMvs(myMove* lpmvs) const;       
    // ��������
    int genMvsWithoutEat(myMove* lpmvs) const;  
    // ȫ���ŷ�
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