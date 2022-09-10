#include "define.h"
#include "pregen.h"
#include "position.h"
#include "preeval.h"
#include"my_evaluate.h"

const int EVAL_MARGIN1 = 160;
const int EVAL_MARGIN2 = 80;
const int EVAL_MARGIN3 = 40;
const int EVAL_MARGIN4 = 20;

const int WHITE_KING_BITFILE = 1 << (RANK_BOTTOM - RANK_TOP);
const int BLACK_KING_BITFILE = 1 << (RANK_TOP - RANK_TOP);
const int KING_BITRANK = 1 << (FILE_CENTER - FILE_LEFT);

const int SHAPE_NONE = 0;
const int SHAPE_CENTER = 1;
const int SHAPE_LEFT = 2;
const int SHAPE_RIGHT = 3;

int myPosition::AdvisorShape(void) const {
    int pcCannon, pcRook, sq, sqAdv1, sqAdv2, x, y, nShape;
    int vlWhitePenalty, vlBlackPenalty;
    mySlideMask* lpsms;
    vlWhitePenalty = vlBlackPenalty = 0;
    if ((this->wBitPiece[0] & OFFIER_BITPIECE) == OFFIER_BITPIECE) {
        if (this->ucsqPieces[whichSide(0) + GENERAL_FROM] == 0xc7) {
            sqAdv1 = this->ucsqPieces[whichSide(0) + OFFIER_FROM];
            sqAdv2 = this->ucsqPieces[whichSide(0) + OFFIER_TO];
            if (false) {
            }
            else if (sqAdv1 == 0xc6) { // 红方一个仕在左侧底线
                nShape = (sqAdv2 == 0xc8 ? SHAPE_CENTER : sqAdv2 == 0xb7 ? SHAPE_LEFT : SHAPE_NONE);
            }
            else if (sqAdv1 == 0xc8) { // 红方一个仕在右侧底线
                nShape = (sqAdv2 == 0xc6 ? SHAPE_CENTER : sqAdv2 == 0xb7 ? SHAPE_RIGHT : SHAPE_NONE);
            }
            else if (sqAdv1 == 0xb7) { // 红方一个仕在花心
                nShape = (sqAdv2 == 0xc6 ? SHAPE_LEFT : sqAdv2 == 0xc8 ? SHAPE_RIGHT : SHAPE_NONE);
            }
            else {
                nShape = SHAPE_NONE;
            }
            switch (nShape) {
            case SHAPE_NONE:
                break;
            case SHAPE_CENTER:
                for (pcCannon = whichSide(1) + CANNON_FROM; pcCannon <= whichSide(1) + CANNON_TO; pcCannon++) {
                    sq = this->ucsqPieces[pcCannon];
                    if (sq != 0) {
                        x = FILE_X(sq);
                        if (x == FILE_CENTER) {
                            y = RANK_Y(sq);
                            lpsms = this->FileMaskPtr(x, y);
                            if ((lpsms->wRookCap & WHITE_KING_BITFILE) != 0) {
                                // 计算空头炮的威胁
                                vlWhitePenalty += PreEvalEx.vlHollowThreat[RANK_FLIP(y)];
                            }
                            else if ((lpsms->wSuperCap & WHITE_KING_BITFILE) != 0 &&
                                (this->ucpcSquares[0xb7] == 21 || this->ucpcSquares[0xb7] == 22)) {
                                // 计算炮镇窝心马的威胁
                                vlWhitePenalty += PreEvalEx.vlCentralThreat[RANK_FLIP(y)];
                            }
                        }
                    }
                }
                break;
            case SHAPE_LEFT:
            case SHAPE_RIGHT:
                for (pcCannon = whichSide(1) + CANNON_FROM; pcCannon <= whichSide(1) + CANNON_TO; pcCannon++) {
                    sq = this->ucsqPieces[pcCannon];
                    if (sq != 0) {
                        x = FILE_X(sq);
                        y = RANK_Y(sq);
                        if (x == FILE_CENTER) {
                            if ((this->FileMaskPtr(x, y)->wSuperCap & WHITE_KING_BITFILE) != 0) {
                                // 计算一般中炮的威胁，帅(将)门被对方控制的还有额外罚分
                                vlWhitePenalty += (PreEvalEx.vlCentralThreat[RANK_FLIP(y)] >> 2) +
                                    (this->ifProtected(1, nShape == SHAPE_LEFT ? 0xc8 : 0xc6) ? 20 : 0);
                                // 如果车在底线保护帅(将)，则给予更大的罚分！
                                for (pcRook = whichSide(0) + JU_FROM; pcRook <= whichSide(0) + JU_TO; pcRook++) {
                                    sq = this->ucsqPieces[pcRook];
                                    if (sq != 0) {
                                        y = RANK_Y(sq);
                                        if (y == RANK_BOTTOM) {
                                            x = FILE_X(sq);
                                            if ((this->RankMaskPtr(x, y)->wRookCap & KING_BITRANK) != 0) {
                                                vlWhitePenalty += 80;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (y == RANK_BOTTOM) {
                            if ((this->RankMaskPtr(x, y)->wRookCap & KING_BITRANK) != 0) {
                                // 计算沉底炮的威胁
                                vlWhitePenalty += PreEvalEx.vlWhiteBottomThreat[x];
                            }
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
        else if (this->ucsqPieces[whichSide(0) + GENERAL_FROM] == 0xb7) {
            // 有双仕(士)但花心被帅(将)占领，要罚分
            vlWhitePenalty += 20;
        }
    }
    else {
        if ((this->wBitPiece[1] & JU_BITPIECE) == JU_BITPIECE) {
            // 缺仕(士)怕双车，有罚分
            vlWhitePenalty += PreEvalEx.vlWhiteAdvisorLeakage;
        }
    }
    if ((this->wBitPiece[1] & OFFIER_BITPIECE) == OFFIER_BITPIECE) {
        if (this->ucsqPieces[whichSide(1) + GENERAL_FROM] == 0x37) {
            sqAdv1 = this->ucsqPieces[whichSide(1) + OFFIER_FROM];
            sqAdv2 = this->ucsqPieces[whichSide(1) + OFFIER_TO];
            if (false) {
            }
            else if (sqAdv1 == 0x36) { // 黑方一个士在左侧底线
                nShape = (sqAdv2 == 0x38 ? SHAPE_CENTER : sqAdv2 == 0x47 ? SHAPE_LEFT : SHAPE_NONE);
            }
            else if (sqAdv1 == 0x38) { // 黑方一个士在右侧底线
                nShape = (sqAdv2 == 0x36 ? SHAPE_CENTER : sqAdv2 == 0x47 ? SHAPE_RIGHT : SHAPE_NONE);
            }
            else if (sqAdv1 == 0x47) { // 黑方一个士在花心
                nShape = (sqAdv2 == 0x36 ? SHAPE_LEFT : sqAdv2 == 0x38 ? SHAPE_RIGHT : SHAPE_NONE);
            }
            else {
                nShape = SHAPE_NONE;
            }
            switch (nShape) {
            case SHAPE_NONE:
                break;
            case SHAPE_CENTER:
                for (pcCannon = whichSide(0) + CANNON_FROM; pcCannon <= whichSide(0) + CANNON_TO; pcCannon++) {
                    sq = this->ucsqPieces[pcCannon];
                    if (sq != 0) {
                        x = FILE_X(sq);
                        if (x == FILE_CENTER) {
                            y = RANK_Y(sq);
                            lpsms = this->FileMaskPtr(x, y);
                            if ((lpsms->wRookCap & BLACK_KING_BITFILE) != 0) {
                                // 计算空头炮的威胁
                                vlBlackPenalty += PreEvalEx.vlHollowThreat[y];
                            }
                            else if ((lpsms->wSuperCap & BLACK_KING_BITFILE) != 0 &&
                                (this->ucpcSquares[0x47] == 37 || this->ucpcSquares[0x47] == 38)) {
                                // 计算炮镇窝心马的威胁
                                vlBlackPenalty += PreEvalEx.vlCentralThreat[y];
                            }
                        }
                    }
                }
                break;
            case SHAPE_LEFT:
            case SHAPE_RIGHT:
                for (pcCannon = whichSide(0) + CANNON_FROM; pcCannon <= whichSide(0) + CANNON_TO; pcCannon++) {
                    sq = this->ucsqPieces[pcCannon];
                    if (sq != 0) {
                        x = FILE_X(sq);
                        y = RANK_Y(sq);
                        if (x == FILE_CENTER) {
                            if ((this->FileMaskPtr(x, y)->wSuperCap & BLACK_KING_BITFILE) != 0) {
                                // 计算一般中炮的威胁，帅(将)门被对方控制的还有额外罚分
                                vlBlackPenalty += (PreEvalEx.vlCentralThreat[y] >> 2) +
                                    (this->ifProtected(0, nShape == SHAPE_LEFT ? 0x38 : 0x36) ? 20 : 0);
                                // 如果车在底线保护帅(将)，则给予更大的罚分！
                                for (pcRook = whichSide(1) + JU_FROM; pcRook <= whichSide(1) + JU_TO; pcRook++) {
                                    sq = this->ucsqPieces[pcRook];
                                    if (sq != 0) {
                                        y = RANK_Y(sq);
                                        if (y == RANK_TOP) {
                                            x = FILE_X(sq);
                                            if ((this->RankMaskPtr(x, y)->wRookCap & KING_BITRANK) != 0) {
                                                vlBlackPenalty += 80;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (y == RANK_TOP) {
                            if ((this->RankMaskPtr(x, y)->wRookCap & KING_BITRANK) != 0) {
                                // 计算沉底炮的威胁
                                vlBlackPenalty += PreEvalEx.vlBlackBottomThreat[x];
                            }
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
        else if (this->ucsqPieces[whichSide(1) + GENERAL_FROM] == 0x47) {
            // 有双仕(士)但花心被帅(将)占领，要罚分
            vlBlackPenalty += 20;
        }
    }
    else {
        if ((this->wBitPiece[0] & JU_BITPIECE) == JU_BITPIECE) {
            // 缺仕(士)怕双车，有罚分
            vlBlackPenalty += PreEvalEx.vlBlackAdvisorLeakage;
        }
    }
    return scoreOfSide(this->nowSide, vlBlackPenalty - vlWhitePenalty);
}

// 局面评价过程
int myPosition::Evaluate(int vlAlpha, int vlBeta) const {
    return outer_Evaluate(nowSide, ucpcSquares, ucsqPieces, dwBitPiece, redScore, blackScore,this);
}