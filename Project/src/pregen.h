#include "define.h"
#include "rc4prng.h"

#ifndef PREGEN_H
#define PREGEN_H

const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3;
const int FILE_CENTER = 7;
const int FILE_RIGHT = 11;

extern const bool cbcInBoard[256];    // ���������
extern const bool cbcInFort[256];     // �ǳ������
extern const bool cbcCanPromote[256]; // ���������
extern const int8_t ccLegalSpanTab[512];   // �����ŷ���ȱ�
extern const int8_t ccKnightPinTab[512];   // ���ȱ�

inline bool IN_BOARD(int sq) {
    return cbcInBoard[sq];
}

inline bool IN_FORT(int sq) {
    return cbcInFort[sq];
}

inline bool CAN_PROMOTE(int sq) {
    return cbcCanPromote[sq];
}

inline int8_t LEGAL_SPAN_TAB(int nDisp) {
    return ccLegalSpanTab[nDisp];
}

inline int8_t KNIGHT_PIN_TAB(int nDisp) {
    return ccKnightPinTab[nDisp];
}

inline int RANK_Y(int sq) {
    return sq >> 4;
}

inline int FILE_X(int sq) {
    return sq & 15;
}

inline int COORD_XY(int x, int y) {
    return x + (y << 4);
}

inline int SQUARE_FLIP(int sq) {
    return 254 - sq;
}

inline int FILE_FLIP(int x) {
    return 14 - x;
}

inline int RANK_FLIP(int y) {
    return 15 - y;
}

inline int OPP_SIDE(int sd) {
    return 1 - sd;
}

inline int SQUARE_FORWARD(int sq, int sd) {
    return sq - 16 + (sd << 5);
}

inline int SQUARE_BACKWARD(int sq, int sd) {
    return sq + 16 - (sd << 5);
}

inline bool KING_SPAN(int sqSrc, int sqDst) {
    return LEGAL_SPAN_TAB(sqDst - sqSrc + 256) == 1;
}

inline bool ADVISOR_SPAN(int sqSrc, int sqDst) {
    return LEGAL_SPAN_TAB(sqDst - sqSrc + 256) == 2;
}

inline bool BISHOP_SPAN(int sqSrc, int sqDst) {
    return LEGAL_SPAN_TAB(sqDst - sqSrc + 256) == 3;
}

inline int BISHOP_PIN(int sqSrc, int sqDst) {
    return (sqSrc + sqDst) >> 1;
}

inline int KNIGHT_PIN(int sqSrc, int sqDst) {
    return sqSrc + KNIGHT_PIN_TAB(sqDst - sqSrc + 256);
}

inline bool WHITE_HALF(int sq) {
    return (sq & 0x80) != 0;
}

inline bool BLACK_HALF(int sq) {
    return (sq & 0x80) == 0;
}

inline bool HOME_HALF(int sq, int sd) {
    return (sq & 0x80) != (sd << 7);
}

inline bool AWAY_HALF(int sq, int sd) {
    return (sq & 0x80) == (sd << 7);
}

inline bool SAME_HALF(int sqSrc, int sqDst) {
    return ((sqSrc ^ sqDst) & 0x80) == 0;
}

inline bool DIFF_HALF(int sqSrc, int sqDst) {
    return ((sqSrc ^ sqDst) & 0x80) != 0;
}

inline int RANK_DISP(int y) {
    return y << 4;
}

inline int FILE_DISP(int x) {
    return x;
}

// ������λ�С��͡�λ�С����ɳ����ŷ���Ԥ�ýṹ
struct mySlideMove {
    uint8_t ucNonCap[2];    // ���������ߵ������һ��/��Сһ��
    uint8_t ucRookCap[2];   // ���������ߵ������һ��/��Сһ��
    uint8_t ucCannonCap[2]; // �ڳ������ߵ������һ��/��Сһ��
    uint8_t ucSuperCap[2];  // ������(�����ӳ���)���ߵ������һ��/��Сһ��
}; // smv

// ������λ�С��͡�λ�С��жϳ����ŷ������Ե�Ԥ�ýṹ
struct mySlideMask {
    uint16_t wNonCap, wRookCap, wCannonCap, wSuperCap;
}; // sms

struct myZobrist {
    uint32_t dwKey, dwLock0, dwLock1;
    void InitZero(void) {
        dwKey = dwLock0 = dwLock1 = 0;
    }
    void InitRC4(myTool& rc4) {
        dwKey = rc4.NextLong();
        dwLock0 = rc4.NextLong();
        dwLock1 = rc4.NextLong();
    }
    void Xor(const myZobrist& zobr) {
        dwKey ^= zobr.dwKey;
        dwLock0 ^= zobr.dwLock0;
        dwLock1 ^= zobr.dwLock1;
    }
    void Xor(const myZobrist& zobr1, const myZobrist& zobr2) {
        dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
        dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
        dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
    }
}; // zobr

extern struct PreGenStruct {
    // Zobrist��ֵ����Zobrist��ֵ��ZobristУ����������
    myZobrist zobrPlayer;
    myZobrist zobrTable[14][256];

    uint16_t wBitRankMask[256]; // ÿ�����ӵ�λ�е�����λ
    uint16_t wBitFileMask[256]; // ÿ�����ӵ�λ�е�����λ

    mySlideMove smvRankMoveTab[9][512];   // 36,864 �ֽ�
    mySlideMove smvFileMoveTab[10][1024]; // 81,920 �ֽ�
    mySlideMask smsRankMaskTab[9][512];   // 36,864 �ֽ�
    mySlideMask smsFileMaskTab[10][1024]; // 81,920 �ֽ�

    uint8_t ucsqKingMoves[256][8];
    uint8_t ucsqAdvisorMoves[256][8];
    uint8_t ucsqBishopMoves[256][8];
    uint8_t ucsqBishopPins[256][4];
    uint8_t ucsqKnightMoves[256][12];
    uint8_t ucsqKnightPins[256][8];
    uint8_t ucsqPawnMoves[2][256][4];
} PreGen;

// ����Ԥ���۽ṹ
extern struct PreEvalStruct {
    bool bPromotion;
    int vlAdvanced;
    uint8_t vlWhitePieces[7][256];
    uint8_t vlBlackPieces[7][256];
} PreEval;

void PreGenInit(void);

#endif