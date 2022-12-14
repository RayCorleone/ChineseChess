#include <string.h>
#include "define.h"
#include "pregen.h"

const bool cbcInBoard[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool cbcInFort[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const bool cbcCanPromote[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int8_t ccLegalSpanTab[512] = {
                       0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
};

const int8_t ccKnightPinTab[512] = {
                               0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0
};

PreGenStruct PreGen;
PreEvalStruct PreEval;

// ????????????????????????????????????????????????sqDst = sqSrc + cnKnightMoveTab[i]
static const int cnKingMoveTab[4]    = {-0x10, -0x01, +0x01, +0x10};
static const int cnAdvisorMoveTab[4] = {-0x11, -0x0f, +0x0f, +0x11};
static const int cnBishopMoveTab[4]  = {-0x22, -0x1e, +0x1e, +0x22};
static const int cnKnightMoveTab[8]  = {-0x21, -0x1f, -0x12, -0x0e, +0x0e, +0x12, +0x1f, +0x21};

void PreGenInit(void) {
    int i, j, k, n, sqSrc, sqDst;
    myTool rc4;
    mySlideMove smv;
    mySlideMask sms;

    // ??????????Zobrist??????
    rc4.InitZero();
    PreGen.zobrPlayer.InitRC4(rc4);
    for (i = 0; i < 14; i++) {
        for (j = 0; j < 256; j++) {
            PreGen.zobrTable[i][j].InitRC4(rc4);
        }
    }

    // ????????????????????????????
    // ??????????????????????????????????????????????????"+/- RANK_TOP/FILE_LEFT"
    for (sqSrc = 0; sqSrc < 256; sqSrc++) {
        if (IN_BOARD(sqSrc)) {
            PreGen.wBitRankMask[sqSrc] = 1 << (FILE_X(sqSrc) - FILE_LEFT);
            PreGen.wBitFileMask[sqSrc] = 1 << (RANK_Y(sqSrc) - RANK_TOP);
        }
        else {
            PreGen.wBitRankMask[sqSrc] = 0;
            PreGen.wBitFileMask[sqSrc] = 0;
        }
    }

    // ??????????????????????????
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 512; j++) {
            // ????????????????????????????????????????????????????????????????
            // 1. ??????????????"SlideMoveTab"????????????????????????????
            smv.ucNonCap[0] = smv.ucNonCap[1] = smv.ucRookCap[0] = smv.ucRookCap[1] =
                smv.ucCannonCap[0] = smv.ucCannonCap[1] = smv.ucSuperCap[0] = smv.ucSuperCap[1] = i + FILE_LEFT;
            sms.wNonCap = sms.wRookCap = sms.wCannonCap = sms.wSuperCap = 0;
            // ??????????"pregen.h"??...[0]??????????????????????????????????[0]??????????
            // 2. ??????????????????????????...[0]??
            for (k = i + 1; k <= 8; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucRookCap[0] = FILE_DISP(k + FILE_LEFT);
                    sms.wRookCap |= 1 << k;
                    break;
                }
                smv.ucNonCap[0] = FILE_DISP(k + FILE_LEFT);
                sms.wNonCap |= 1 << k;
            }
            for (k++; k <= 8; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucCannonCap[0] = FILE_DISP(k + FILE_LEFT);
                    sms.wCannonCap |= 1 << k;
                    break;
                }
            }
            for (k++; k <= 8; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucSuperCap[0] = FILE_DISP(k + FILE_LEFT);
                    sms.wSuperCap |= 1 << k;
                    break;
                }
            }
            // 3. ??????????????????????????...[1]
            for (k = i - 1; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucRookCap[1] = FILE_DISP(k + FILE_LEFT);
                    sms.wRookCap |= 1 << k;
                    break;
                }
                smv.ucNonCap[1] = FILE_DISP(k + FILE_LEFT);
                sms.wNonCap |= 1 << k;
            }
            for (k--; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucCannonCap[1] = FILE_DISP(k + FILE_LEFT);
                    sms.wCannonCap |= 1 << k;
                    break;
                }
            }
            for (k--; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucSuperCap[1] = FILE_DISP(k + FILE_LEFT);
                    sms.wSuperCap |= 1 << k;
                    break;
                }
            }
            // 5. ??????????"smv"??"sms"??????????????????????
            PreGen.smvRankMoveTab[i][j] = smv;
            PreGen.smsRankMaskTab[i][j] = sms;
        }
    }

    // ??????????????????????????
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 1024; j++) {
            // ????????????????????????????????????????????????????????????????
            // 1. ??????????????"smv"????????????????????????????
            smv.ucNonCap[0] = smv.ucNonCap[1] = smv.ucRookCap[0] = smv.ucRookCap[1] =
                smv.ucCannonCap[0] = smv.ucCannonCap[1] = smv.ucSuperCap[0] = smv.ucSuperCap[1] = (i + RANK_TOP) * 16;
            sms.wNonCap = sms.wRookCap = sms.wCannonCap = sms.wSuperCap = 0;
            // 2. ??????????????????????????...[0]
            for (k = i + 1; k <= 9; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucRookCap[0] = RANK_DISP(k + RANK_TOP);
                    sms.wRookCap |= 1 << k;
                    break;
                }
                smv.ucNonCap[0] = RANK_DISP(k + RANK_TOP);
                sms.wNonCap |= 1 << k;
            }
            for (k++; k <= 9; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucCannonCap[0] = RANK_DISP(k + RANK_TOP);
                    sms.wCannonCap |= 1 << k;
                    break;
                }
            }
            for (k++; k <= 9; k++) {
                if ((j & (1 << k)) != 0) {
                    smv.ucSuperCap[0] = RANK_DISP(k + RANK_TOP);
                    sms.wSuperCap |= 1 << k;
                    break;
                }
            }
            // 3. ??????????????????????????...[1]
            for (k = i - 1; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucRookCap[1] = RANK_DISP(k + RANK_TOP);
                    sms.wRookCap |= 1 << k;
                    break;
                }
                smv.ucNonCap[1] = RANK_DISP(k + RANK_TOP);
                sms.wNonCap |= 1 << k;
            }
            for (k--; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucCannonCap[1] = RANK_DISP(k + RANK_TOP);
                    sms.wCannonCap |= 1 << k;
                    break;
                }
            }
            for (k--; k >= 0; k--) {
                if ((j & (1 << k)) != 0) {
                    smv.ucSuperCap[1] = RANK_DISP(k + RANK_TOP);
                    sms.wSuperCap |= 1 << k;
                    break;
                }
            }
            // 5. ??????????"smv"??"sms"??????????????????????
            PreGen.smvFileMoveTab[i][j] = smv;
            PreGen.smsFileMaskTab[i][j] = sms;
        }
    }

    // ??????????????????????????????????????????
    for (sqSrc = 0; sqSrc < 256; sqSrc++) {
        if (IN_BOARD(sqSrc)) {
            // ??????(??)????????????????
            n = 0;
            for (i = 0; i < 4; i++) {
                sqDst = sqSrc + cnKingMoveTab[i];
                if (IN_FORT(sqDst)) {
                    PreGen.ucsqKingMoves[sqSrc][n] = sqDst;
                    n++;
                }
            }
            PreGen.ucsqKingMoves[sqSrc][n] = 0;
            // ??????(??)????????????????
            n = 0;
            for (i = 0; i < 4; i++) {
                sqDst = sqSrc + cnAdvisorMoveTab[i];
                if (IN_FORT(sqDst)) {
                    PreGen.ucsqAdvisorMoves[sqSrc][n] = sqDst;
                    n++;
                }
            }
            PreGen.ucsqAdvisorMoves[sqSrc][n] = 0;
            // ??????(??)??????????????????????????????
            n = 0;
            for (i = 0; i < 4; i++) {
                sqDst = sqSrc + cnBishopMoveTab[i];
                if (IN_BOARD(sqDst) && SAME_HALF(sqSrc, sqDst)) {
                    PreGen.ucsqBishopMoves[sqSrc][n] = sqDst;
                    PreGen.ucsqBishopPins[sqSrc][n] = BISHOP_PIN(sqSrc, sqDst);
                    n++;
                }
            }
            PreGen.ucsqBishopMoves[sqSrc][n] = 0;
            // ????????????????????????????????????
            n = 0;
            for (i = 0; i < 8; i++) {
                sqDst = sqSrc + cnKnightMoveTab[i];
                if (IN_BOARD(sqDst)) {
                    PreGen.ucsqKnightMoves[sqSrc][n] = sqDst;
                    PreGen.ucsqKnightPins[sqSrc][n] = KNIGHT_PIN(sqSrc, sqDst);
                    n++;
                }
            }
            PreGen.ucsqKnightMoves[sqSrc][n] = 0;
            // ??????(??)????????????????
            for (i = 0; i < 2; i++) {
                n = 0;
                sqDst = SQUARE_FORWARD(sqSrc, i);
                sqDst = sqSrc + (i == 0 ? -16 : 16);
                if (IN_BOARD(sqDst)) {
                    PreGen.ucsqPawnMoves[i][sqSrc][n] = sqDst;
                    n++;
                }
                if (AWAY_HALF(sqSrc, i)) {
                    for (j = -1; j <= 1; j += 2) {
                        sqDst = sqSrc + j;
                        if (IN_BOARD(sqDst)) {
                            PreGen.ucsqPawnMoves[i][sqSrc][n] = sqDst;
                            n++;
                        }
                    }
                }
                PreGen.ucsqPawnMoves[i][sqSrc][n] = 0;
            }
        }
    }

    // ??????????????????????
    memset(&PreEval, 0, sizeof(PreEvalStruct));
    PreEval.bPromotion = false; // ??????????????????
}