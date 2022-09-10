#include "define.h"
#include "pregen.h"
#include "position.h"

// 棋子保护判断
bool myPosition::ifProtected(int sd, int sqSrc, int sqExcept) const {
    // 参数"sqExcept"表示排除保护的棋子(指格子编号)，考虑被牵制子的保护时，需要排除牵制目标子的保护
    int i, sqDst, sqPin, pc, x, y, nSideTag;
    mySlideMask* lpsmsRank, * lpsmsFile;
    // 棋子保护判断包括以下几个步骤：

    nSideTag = whichSide(sd);
    if (HOME_HALF(sqSrc, sd)) {
        if (IN_FORT(sqSrc)) {

            // 1. 判断受到帅(将)的保护
            sqDst = ucsqPieces[nSideTag + GENERAL_FROM];
            if (sqDst != 0 && sqDst != sqExcept) {
                if (KING_SPAN(sqSrc, sqDst)) {
                    return true;
                }
            }

            // 2. 判断受到仕(士)的保护
            for (i = OFFIER_FROM; i <= OFFIER_TO; i++) {
                sqDst = ucsqPieces[nSideTag + i];
                if (sqDst != 0 && sqDst != sqExcept) {
                    if (ADVISOR_SPAN(sqSrc, sqDst)) {
                        return true;
                    }
                }
            }
        }

        // 3. 判断受到相(象)的保护
        for (i = ELE_FROM; i <= ELE_TO; i++) {
            sqDst = ucsqPieces[nSideTag + i];
            if (sqDst != 0 && sqDst != sqExcept) {
                if (BISHOP_SPAN(sqSrc, sqDst) && ucpcSquares[BISHOP_PIN(sqSrc, sqDst)] == 0) {
                    return true;
                }
            }
        }
    }
    else {

        // 4. 判断受到过河兵(卒)横向的保护
        for (sqDst = sqSrc - 1; sqDst <= sqSrc + 1; sqDst += 2) {
            // 如果棋子在边线，那么断言不成立
            // __ASSERT_SQUARE(sqDst);
            if (sqDst != sqExcept) {
                pc = ucpcSquares[sqDst];
                if ((pc & nSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
                    return true;
                }
            }
        }
    }

    // 5. 判断受到兵(卒)纵向的保护
    sqDst = SQUARE_BACKWARD(sqSrc, sd);
    // 如果棋子在底线，那么断言不成立
    // __ASSERT_SQUARE(sqDst);
    if (sqDst != sqExcept) {
        pc = ucpcSquares[sqDst];
        if ((pc & nSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
            return true;
        }
    }

    // 6. 判断受到马的保护
    for (i = H_FROM; i <= H_TO; i++) {
        sqDst = ucsqPieces[nSideTag + i];
        if (sqDst != 0 && sqDst != sqExcept) {
            sqPin = KNIGHT_PIN(sqDst, sqSrc); // 注意，sqSrc和sqDst是反的！
            if (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                return true;
            }
        }
    }

    x = FILE_X(sqSrc);
    y = RANK_Y(sqSrc);
    lpsmsRank = RankMaskPtr(x, y);
    lpsmsFile = FileMaskPtr(x, y);

    // 7. 判断受到车的保护，参阅"position.cpp"里的"getCheckingOne()"函数
    for (i = JU_FROM; i <= JU_TO; i++) {
        sqDst = ucsqPieces[nSideTag + i];
        if (sqDst != 0 && sqDst != sqSrc && sqDst != sqExcept) {
            if (x == FILE_X(sqDst)) {
                if ((lpsmsFile->wRookCap & PreGen.wBitFileMask[sqDst]) != 0) {
                    return true;
                }
            }
            else if (y == RANK_Y(sqDst)) {
                if ((lpsmsRank->wRookCap & PreGen.wBitRankMask[sqDst]) != 0) {
                    return true;
                }
            }
        }
    }

    // 8. 判断受到炮的保护，参阅"position.cpp"里的"getCheckingOne()"函数
    for (i = CANNON_FROM; i <= CANNON_TO; i++) {
        sqDst = ucsqPieces[nSideTag + i];
        if (sqDst && sqDst != sqSrc && sqDst != sqExcept) {
            if (x == FILE_X(sqDst)) {
                if ((lpsmsFile->wCannonCap & PreGen.wBitFileMask[sqDst]) != 0) {
                    return true;
                }
            }
            else if (y == RANK_Y(sqDst)) {
                if ((lpsmsRank->wCannonCap & PreGen.wBitRankMask[sqDst]) != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

int myPosition::MvvLva(int sqDst, int pcCaptured, int nLva) const {
    int nMvv, nLvaAdjust;
    nMvv = baScore(pcCaptured);
    nLvaAdjust = (ifProtected(OPP_SIDE(nowSide), sqDst) ? nLva : 0);
    if (nMvv >= nLvaAdjust) {
        return nMvv - nLvaAdjust + 1;
    }
    else {
        return (nMvv >= 3 || HOME_HALF(sqDst, nowSide)) ? 1 : 0;
    }
}

// 吃子着法生成器，按MVV(LVA)设定分值
int myPosition::genEatMvs(myMove* lpmvs) const {
    int i, sqSrc, sqDst, pcCaptured;
    int x, y, nSideTag, nOppSideTag;
    bool bCanPromote;
    mySlideMove* lpsmv;
    uint8_t* lpucsqDst, * lpucsqPin;
    myMove* lpmvsCurr;
    // 生成吃子着法的过程包括以下几个步骤：

    lpmvsCurr = lpmvs;
    nSideTag = whichSide(nowSide);
    nOppSideTag = resWhichSide(nowSide);
    bCanPromote = PreEval.bPromotion && ifPassed();

    // 1. 生成帅(将)的着法
    sqSrc = ucsqPieces[nSideTag + GENERAL_FROM];
    if (sqSrc != 0) {
        lpucsqDst = PreGen.ucsqKingMoves[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            // 找到一个着法后，首先判断吃到的棋子是否是对方棋子，技巧是利用"nOppSideTag"的标志(16和32颠倒)，
            // 如果是对方棋子，则保存MVV(LVA)值，即如果被吃子无保护，则只记MVV，否则记MVV-LVA(如果MVV>LVA的话)。
            pcCaptured = ucpcSquares[sqDst];
            if ((pcCaptured & nOppSideTag) != 0) {
                lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 5); // 帅(将)的价值是5
                lpmvsCurr++;
            }
            lpucsqDst++;
            sqDst = *lpucsqDst;
        }
    }

    // 2. 生成仕(士)的着法
    for (i = OFFIER_FROM; i <= OFFIER_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqAdvisorMoves[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 1); // 仕(士)的价值是1
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
            }
            if (bCanPromote && CAN_PROMOTE(sqSrc)) {
                lpmvsCurr->bmv = getMbySD(sqSrc, sqSrc);
                lpmvsCurr->wsc = 0;
                lpmvsCurr++;
            }
        }
    }

    // 3. 生成相(象)的着法
    for (i = ELE_FROM; i <= ELE_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqBishopMoves[sqSrc];
            lpucsqPin = PreGen.ucsqBishopPins[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[*lpucsqPin] == 0) {
                    pcCaptured = ucpcSquares[sqDst];
                    if ((pcCaptured & nOppSideTag) != 0) {
                        lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                        lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 1); // 相(象)的价值是1
                        lpmvsCurr++;
                    }
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
                lpucsqPin++;
            }
            if (bCanPromote && CAN_PROMOTE(sqSrc)) {
                lpmvsCurr->bmv = getMbySD(sqSrc, sqSrc);
                lpmvsCurr->wsc = 0;
                lpmvsCurr++;
            }
        }
    }

    // 4. 生成马的着法
    for (i = H_FROM; i <= H_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqKnightMoves[sqSrc];
            lpucsqPin = PreGen.ucsqKnightPins[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[*lpucsqPin] == 0) {
                    pcCaptured = ucpcSquares[sqDst];
                    if ((pcCaptured & nOppSideTag) != 0) {
                        lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                        lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // 马的价值是3
                        lpmvsCurr++;
                    }
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
                lpucsqPin++;
            }
        }
    }

    // 5. 生成车的着法
    for (i = JU_FROM; i <= JU_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            x = FILE_X(sqSrc);
            y = RANK_Y(sqSrc);

            lpsmv = RankMovePtr(x, y);
            sqDst = lpsmv->ucRookCap[0] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // 车的价值是4
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucRookCap[1] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // 车的价值是4
                    lpmvsCurr++;
                }
            }

            lpsmv = FileMovePtr(x, y);
            sqDst = lpsmv->ucRookCap[0] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // 车的价值是4
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucRookCap[1] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // 车的价值是4
                    lpmvsCurr++;
                }
            }
        }
    }

    // 6. 生成炮的着法
    for (i = CANNON_FROM; i <= CANNON_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            x = FILE_X(sqSrc);
            y = RANK_Y(sqSrc);

            lpsmv = RankMovePtr(x, y);
            sqDst = lpsmv->ucCannonCap[0] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // 炮的价值是3
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucCannonCap[1] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // 炮的价值是3
                    lpmvsCurr++;
                }
            }

            lpsmv = FileMovePtr(x, y);
            sqDst = lpsmv->ucCannonCap[0] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // 炮的价值是3
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucCannonCap[1] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // 炮的价值是3
                    lpmvsCurr++;
                }
            }
        }
    }

    // 7. 生成兵(卒)的着法
    for (i = PAWN_FROM; i <= PAWN_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqPawnMoves[nowSide][sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 2); // 兵(卒)的价值是2
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
            }
        }
    }
    return lpmvsCurr - lpmvs;
}

// 不吃子着法生成器
int myPosition::genMvsWithoutEat(myMove* lpmvs) const {
    int i, sqSrc, sqDst, x, y, nSideTag;
    mySlideMove* lpsmv;
    uint8_t* lpucsqDst, * lpucsqPin;
    myMove* lpmvsCurr;
    // 生成不吃子着法的过程包括以下几个步骤：

    lpmvsCurr = lpmvs;
    nSideTag = whichSide(nowSide);

    // 1. 生成帅(将)的着法
    sqSrc = ucsqPieces[nSideTag + GENERAL_FROM];
    if (sqSrc != 0) {
        lpucsqDst = PreGen.ucsqKingMoves[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            // 找到一个着法后，首先判断是否吃到棋子
            if (ucpcSquares[sqDst] == 0) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
            }
            lpucsqDst++;
            sqDst = *lpucsqDst;
        }
    }

    // 2. 生成仕(士)的着法
    for (i = OFFIER_FROM; i <= OFFIER_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqAdvisorMoves[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[sqDst] == 0) {
                    lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
            }
        }
    }

    // 3. 生成相(象)的着法
    for (i = ELE_FROM; i <= ELE_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqBishopMoves[sqSrc];
            lpucsqPin = PreGen.ucsqBishopPins[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[*lpucsqPin] == 0 && ucpcSquares[sqDst] == 0) {
                    lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
                lpucsqPin++;
            }
        }
    }

    // 4. 生成马的着法
    for (i = H_FROM; i <= H_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqKnightMoves[sqSrc];
            lpucsqPin = PreGen.ucsqKnightPins[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[*lpucsqPin] == 0 && ucpcSquares[sqDst] == 0) {
                    lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
                lpucsqPin++;
            }
        }
    }

    // 5. 生成车和炮的着法，没有必要判断是否吃到本方棋子
    for (i = JU_FROM; i <= CANNON_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            x = FILE_X(sqSrc);
            y = RANK_Y(sqSrc);

            lpsmv = RankMovePtr(x, y);
            sqDst = lpsmv->ucNonCap[0] + RANK_DISP(y);
            while (sqDst != sqSrc) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
                sqDst--;
            }
            sqDst = lpsmv->ucNonCap[1] + RANK_DISP(y);
            while (sqDst != sqSrc) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
                sqDst++;
            }

            lpsmv = FileMovePtr(x, y);
            sqDst = lpsmv->ucNonCap[0] + FILE_DISP(x);
            while (sqDst != sqSrc) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
                sqDst -= 16;
            }
            sqDst = lpsmv->ucNonCap[1] + FILE_DISP(x);
            while (sqDst != sqSrc) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
                sqDst += 16;
            }
        }
    }

    // 6. 生成兵(卒)的着法
    for (i = PAWN_FROM; i <= PAWN_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqPawnMoves[nowSide][sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                if (ucpcSquares[sqDst] == 0) {
                    lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
            }
        }
    }
    return lpmvsCurr - lpmvs;
}

// “捉”的检测
int myPosition::chasedBy(int mv) const {
    int i, nSideTag, pcMoved, pcCaptured;
    int sqSrc, sqDst, x, y;
    uint8_t* lpucsqDst, * lpucsqPin;
    mySlideMove* lpsmv;

    sqSrc = getDir(mv);
    pcMoved = this->ucpcSquares[sqSrc];
    nSideTag = whichSide(this->nowSide);

    // “捉”的判断包括以下几部分内容：
    switch (pcMoved - resWhichSide(this->nowSide)) {

        // 1. 走了马，判断是否捉车或捉有根的炮兵(卒)
    case H_FROM:
    case H_TO:
        // 逐一检测马踩的八个位置
        lpucsqDst = PreGen.ucsqKnightMoves[sqSrc];
        lpucsqPin = PreGen.ucsqKnightPins[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            if (ucpcSquares[*lpucsqPin] == 0) {
                pcCaptured = this->ucpcSquares[sqDst];
                if ((pcCaptured & nSideTag) != 0) {
                    pcCaptured -= nSideTag;
                    // 技巧：优化兵种判断的分枝
                    if (pcCaptured <= JU_TO) {
                        // 马捉仕(士)、相(象)和马的情况不予考虑
                        if (pcCaptured >= JU_FROM) {
                            // 马捉到了车
                            return pcCaptured;
                        }
                    }
                    else {
                        if (pcCaptured <= CANNON_TO) {
                            // 马捉到了炮，要判断炮是否受保护
                            if (!ifProtected(this->nowSide, sqDst)) {
                                return pcCaptured;
                            }
                        }
                        else {
                            // 马捉到了兵(卒)，要判断兵(卒)是否过河并受保护
                            if (AWAY_HALF(sqDst, nowSide) && !ifProtected(this->nowSide, sqDst)) {
                                return pcCaptured;
                            }
                        }
                    }
                }
            }
            lpucsqDst++;
            sqDst = *lpucsqDst;
            lpucsqPin++;
        }
        break;

        // 2. 走了车，判断是否捉有根的马炮兵(卒)
    case JU_FROM:
    case JU_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (((getSta(mv) ^ sqSrc) & 0xf) == 0) {
            // 如果车纵向移动了，则判断车横向吃到的子
            lpsmv = RankMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucRookCap[i] + RANK_DISP(y);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // 技巧：优化兵种判断的分枝
                        if (pcCaptured <= JU_TO) {
                            // 车捉仕(士)、相(象)的情况不予考虑
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // 车捉到了马，要判断马是否受保护
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                // 车捉车的情况不予考虑
                            }
                        }
                        else {
                            if (pcCaptured <= CANNON_TO) {
                                // 车捉到了炮，要判断炮是否受保护
                                if (!ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                            else {
                                // 车捉到了兵(卒)，要判断兵(卒)是否过河并受保护
                                if (AWAY_HALF(sqDst, nowSide) && !ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            // 如果车横向移动了，则判断车纵向吃到的子
            lpsmv = FileMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucRookCap[i] + FILE_DISP(x);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // 技巧：优化兵种判断的分枝
                        if (pcCaptured <= JU_TO) {
                            // 车捉仕(士)、相(象)的情况不予考虑
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // 车捉到了马，要判断马是否受保护
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                // 车捉车的情况不予考虑
                            }
                        }
                        else {
                            if (pcCaptured <= CANNON_TO) {
                                // 车捉到了炮，要判断炮是否受保护
                                if (!ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                            else {
                                // 车捉到了兵(卒)，要判断兵(卒)是否过河并受保护
                                if (AWAY_HALF(sqDst, nowSide) && !ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                        }
                    }
                }
            }
        }
        break;

        // 3. 走了炮，判断是否捉车或捉有根的马兵(卒)
    case CANNON_FROM:
    case CANNON_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (((getSta(mv) ^ sqSrc) & 0xf) == 0) {
            // 如果炮纵向移动了，则判断炮横向吃到的子
            lpsmv = RankMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucCannonCap[i] + RANK_DISP(y);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // 技巧：优化兵种判断的分枝
                        if (pcCaptured <= JU_TO) {
                            // 炮捉仕(士)、相(象)的情况不予考虑
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // 炮捉到了马，要判断马是否受保护
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                else {
                                    // 炮捉到了车
                                    return pcCaptured;
                                }
                            }
                        }
                        else {
                            // 炮捉炮的情况不予考虑
                            if (pcCaptured >= PAWN_FROM) {
                                // 炮捉到了兵(卒)，要判断兵(卒)是否过河并受保护
                                if (AWAY_HALF(sqDst, nowSide) && !ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            // 如果炮横向移动了，则判断炮纵向吃到的子
            lpsmv = FileMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucCannonCap[i] + FILE_DISP(x);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // 技巧：优化兵种判断的分枝
                        if (pcCaptured <= JU_TO) {
                            // 炮捉仕(士)、相(象)的情况不予考虑
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // 炮捉到了马，要判断马是否受保护
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                else {
                                    // 炮捉到了车
                                    return pcCaptured;
                                }
                            }
                        }
                        else {
                            // 炮捉炮的情况不予考虑
                            if (pcCaptured >= PAWN_FROM) {
                                // 炮捉到了兵(卒)，要判断兵(卒)是否过河并受保护
                                if (AWAY_HALF(sqDst, nowSide) && !ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    }

    return 0;
}