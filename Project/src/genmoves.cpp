#include "define.h"
#include "pregen.h"
#include "position.h"

// ���ӱ����ж�
bool myPosition::ifProtected(int sd, int sqSrc, int sqExcept) const {
    // ����"sqExcept"��ʾ�ų�����������(ָ���ӱ��)�����Ǳ�ǣ���ӵı���ʱ����Ҫ�ų�ǣ��Ŀ���ӵı���
    int i, sqDst, sqPin, pc, x, y, nSideTag;
    mySlideMask* lpsmsRank, * lpsmsFile;
    // ���ӱ����жϰ������¼������裺

    nSideTag = whichSide(sd);
    if (HOME_HALF(sqSrc, sd)) {
        if (IN_FORT(sqSrc)) {

            // 1. �ж��ܵ�˧(��)�ı���
            sqDst = ucsqPieces[nSideTag + GENERAL_FROM];
            if (sqDst != 0 && sqDst != sqExcept) {
                if (KING_SPAN(sqSrc, sqDst)) {
                    return true;
                }
            }

            // 2. �ж��ܵ���(ʿ)�ı���
            for (i = OFFIER_FROM; i <= OFFIER_TO; i++) {
                sqDst = ucsqPieces[nSideTag + i];
                if (sqDst != 0 && sqDst != sqExcept) {
                    if (ADVISOR_SPAN(sqSrc, sqDst)) {
                        return true;
                    }
                }
            }
        }

        // 3. �ж��ܵ���(��)�ı���
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

        // 4. �ж��ܵ����ӱ�(��)����ı���
        for (sqDst = sqSrc - 1; sqDst <= sqSrc + 1; sqDst += 2) {
            // ��������ڱ��ߣ���ô���Բ�����
            // __ASSERT_SQUARE(sqDst);
            if (sqDst != sqExcept) {
                pc = ucpcSquares[sqDst];
                if ((pc & nSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
                    return true;
                }
            }
        }
    }

    // 5. �ж��ܵ���(��)����ı���
    sqDst = SQUARE_BACKWARD(sqSrc, sd);
    // ��������ڵ��ߣ���ô���Բ�����
    // __ASSERT_SQUARE(sqDst);
    if (sqDst != sqExcept) {
        pc = ucpcSquares[sqDst];
        if ((pc & nSideTag) != 0 && pcIn(pc) >= PAWN_FROM) {
            return true;
        }
    }

    // 6. �ж��ܵ���ı���
    for (i = H_FROM; i <= H_TO; i++) {
        sqDst = ucsqPieces[nSideTag + i];
        if (sqDst != 0 && sqDst != sqExcept) {
            sqPin = KNIGHT_PIN(sqDst, sqSrc); // ע�⣬sqSrc��sqDst�Ƿ��ģ�
            if (sqPin != sqDst && ucpcSquares[sqPin] == 0) {
                return true;
            }
        }
    }

    x = FILE_X(sqSrc);
    y = RANK_Y(sqSrc);
    lpsmsRank = RankMaskPtr(x, y);
    lpsmsFile = FileMaskPtr(x, y);

    // 7. �ж��ܵ����ı���������"position.cpp"���"getCheckingOne()"����
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

    // 8. �ж��ܵ��ڵı���������"position.cpp"���"getCheckingOne()"����
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

// �����ŷ�����������MVV(LVA)�趨��ֵ
int myPosition::genEatMvs(myMove* lpmvs) const {
    int i, sqSrc, sqDst, pcCaptured;
    int x, y, nSideTag, nOppSideTag;
    bool bCanPromote;
    mySlideMove* lpsmv;
    uint8_t* lpucsqDst, * lpucsqPin;
    myMove* lpmvsCurr;
    // ���ɳ����ŷ��Ĺ��̰������¼������裺

    lpmvsCurr = lpmvs;
    nSideTag = whichSide(nowSide);
    nOppSideTag = resWhichSide(nowSide);
    bCanPromote = PreEval.bPromotion && ifPassed();

    // 1. ����˧(��)���ŷ�
    sqSrc = ucsqPieces[nSideTag + GENERAL_FROM];
    if (sqSrc != 0) {
        lpucsqDst = PreGen.ucsqKingMoves[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            // �ҵ�һ���ŷ��������жϳԵ��������Ƿ��ǶԷ����ӣ�����������"nOppSideTag"�ı�־(16��32�ߵ�)��
            // ����ǶԷ����ӣ��򱣴�MVV(LVA)ֵ��������������ޱ�������ֻ��MVV�������MVV-LVA(���MVV>LVA�Ļ�)��
            pcCaptured = ucpcSquares[sqDst];
            if ((pcCaptured & nOppSideTag) != 0) {
                lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 5); // ˧(��)�ļ�ֵ��5
                lpmvsCurr++;
            }
            lpucsqDst++;
            sqDst = *lpucsqDst;
        }
    }

    // 2. ������(ʿ)���ŷ�
    for (i = OFFIER_FROM; i <= OFFIER_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqAdvisorMoves[sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 1); // ��(ʿ)�ļ�ֵ��1
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

    // 3. ������(��)���ŷ�
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
                        lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 1); // ��(��)�ļ�ֵ��1
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

    // 4. ��������ŷ�
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
                        lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // ��ļ�ֵ��3
                        lpmvsCurr++;
                    }
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
                lpucsqPin++;
            }
        }
    }

    // 5. ���ɳ����ŷ�
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
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // ���ļ�ֵ��4
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucRookCap[1] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // ���ļ�ֵ��4
                    lpmvsCurr++;
                }
            }

            lpsmv = FileMovePtr(x, y);
            sqDst = lpsmv->ucRookCap[0] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // ���ļ�ֵ��4
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucRookCap[1] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 4); // ���ļ�ֵ��4
                    lpmvsCurr++;
                }
            }
        }
    }

    // 6. �����ڵ��ŷ�
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
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // �ڵļ�ֵ��3
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucCannonCap[1] + RANK_DISP(y);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // �ڵļ�ֵ��3
                    lpmvsCurr++;
                }
            }

            lpsmv = FileMovePtr(x, y);
            sqDst = lpsmv->ucCannonCap[0] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // �ڵļ�ֵ��3
                    lpmvsCurr++;
                }
            }
            sqDst = lpsmv->ucCannonCap[1] + FILE_DISP(x);
            if (sqDst != sqSrc) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 3); // �ڵļ�ֵ��3
                    lpmvsCurr++;
                }
            }
        }
    }

    // 7. ���ɱ�(��)���ŷ�
    for (i = PAWN_FROM; i <= PAWN_TO; i++) {
        sqSrc = ucsqPieces[nSideTag + i];
        if (sqSrc != 0) {
            lpucsqDst = PreGen.ucsqPawnMoves[nowSide][sqSrc];
            sqDst = *lpucsqDst;
            while (sqDst != 0) {
                pcCaptured = ucpcSquares[sqDst];
                if ((pcCaptured & nOppSideTag) != 0) {
                    lpmvsCurr->bmv = getMbySD(sqSrc, sqDst);
                    lpmvsCurr->wsc = MvvLva(sqDst, pcCaptured, 2); // ��(��)�ļ�ֵ��2
                    lpmvsCurr++;
                }
                lpucsqDst++;
                sqDst = *lpucsqDst;
            }
        }
    }
    return lpmvsCurr - lpmvs;
}

// �������ŷ�������
int myPosition::genMvsWithoutEat(myMove* lpmvs) const {
    int i, sqSrc, sqDst, x, y, nSideTag;
    mySlideMove* lpsmv;
    uint8_t* lpucsqDst, * lpucsqPin;
    myMove* lpmvsCurr;
    // ���ɲ������ŷ��Ĺ��̰������¼������裺

    lpmvsCurr = lpmvs;
    nSideTag = whichSide(nowSide);

    // 1. ����˧(��)���ŷ�
    sqSrc = ucsqPieces[nSideTag + GENERAL_FROM];
    if (sqSrc != 0) {
        lpucsqDst = PreGen.ucsqKingMoves[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            // �ҵ�һ���ŷ��������ж��Ƿ�Ե�����
            if (ucpcSquares[sqDst] == 0) {
                lpmvsCurr->dwmv = getMbySD(sqSrc, sqDst);
                lpmvsCurr++;
            }
            lpucsqDst++;
            sqDst = *lpucsqDst;
        }
    }

    // 2. ������(ʿ)���ŷ�
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

    // 3. ������(��)���ŷ�
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

    // 4. ��������ŷ�
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

    // 5. ���ɳ����ڵ��ŷ���û�б�Ҫ�ж��Ƿ�Ե���������
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

    // 6. ���ɱ�(��)���ŷ�
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

// ��׽���ļ��
int myPosition::chasedBy(int mv) const {
    int i, nSideTag, pcMoved, pcCaptured;
    int sqSrc, sqDst, x, y;
    uint8_t* lpucsqDst, * lpucsqPin;
    mySlideMove* lpsmv;

    sqSrc = getDir(mv);
    pcMoved = this->ucpcSquares[sqSrc];
    nSideTag = whichSide(this->nowSide);

    // ��׽�����жϰ������¼��������ݣ�
    switch (pcMoved - resWhichSide(this->nowSide)) {

        // 1. �������ж��Ƿ�׽����׽�и����ڱ�(��)
    case H_FROM:
    case H_TO:
        // ��һ�����ȵİ˸�λ��
        lpucsqDst = PreGen.ucsqKnightMoves[sqSrc];
        lpucsqPin = PreGen.ucsqKnightPins[sqSrc];
        sqDst = *lpucsqDst;
        while (sqDst != 0) {
            if (ucpcSquares[*lpucsqPin] == 0) {
                pcCaptured = this->ucpcSquares[sqDst];
                if ((pcCaptured & nSideTag) != 0) {
                    pcCaptured -= nSideTag;
                    // ���ɣ��Ż������жϵķ�֦
                    if (pcCaptured <= JU_TO) {
                        // ��׽��(ʿ)����(��)�����������迼��
                        if (pcCaptured >= JU_FROM) {
                            // ��׽���˳�
                            return pcCaptured;
                        }
                    }
                    else {
                        if (pcCaptured <= CANNON_TO) {
                            // ��׽�����ڣ�Ҫ�ж����Ƿ��ܱ���
                            if (!ifProtected(this->nowSide, sqDst)) {
                                return pcCaptured;
                            }
                        }
                        else {
                            // ��׽���˱�(��)��Ҫ�жϱ�(��)�Ƿ���Ӳ��ܱ���
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

        // 2. ���˳����ж��Ƿ�׽�и������ڱ�(��)
    case JU_FROM:
    case JU_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (((getSta(mv) ^ sqSrc) & 0xf) == 0) {
            // ����������ƶ��ˣ����жϳ�����Ե�����
            lpsmv = RankMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucRookCap[i] + RANK_DISP(y);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // ���ɣ��Ż������жϵķ�֦
                        if (pcCaptured <= JU_TO) {
                            // ��׽��(ʿ)����(��)��������迼��
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // ��׽������Ҫ�ж����Ƿ��ܱ���
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                // ��׽����������迼��
                            }
                        }
                        else {
                            if (pcCaptured <= CANNON_TO) {
                                // ��׽�����ڣ�Ҫ�ж����Ƿ��ܱ���
                                if (!ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                            else {
                                // ��׽���˱�(��)��Ҫ�жϱ�(��)�Ƿ���Ӳ��ܱ���
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
            // ����������ƶ��ˣ����жϳ�����Ե�����
            lpsmv = FileMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucRookCap[i] + FILE_DISP(x);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // ���ɣ��Ż������жϵķ�֦
                        if (pcCaptured <= JU_TO) {
                            // ��׽��(ʿ)����(��)��������迼��
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // ��׽������Ҫ�ж����Ƿ��ܱ���
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                // ��׽����������迼��
                            }
                        }
                        else {
                            if (pcCaptured <= CANNON_TO) {
                                // ��׽�����ڣ�Ҫ�ж����Ƿ��ܱ���
                                if (!ifProtected(this->nowSide, sqDst)) {
                                    return pcCaptured;
                                }
                            }
                            else {
                                // ��׽���˱�(��)��Ҫ�жϱ�(��)�Ƿ���Ӳ��ܱ���
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

        // 3. �����ڣ��ж��Ƿ�׽����׽�и������(��)
    case CANNON_FROM:
    case CANNON_TO:
        x = FILE_X(sqSrc);
        y = RANK_Y(sqSrc);
        if (((getSta(mv) ^ sqSrc) & 0xf) == 0) {
            // ����������ƶ��ˣ����ж��ں���Ե�����
            lpsmv = RankMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucCannonCap[i] + RANK_DISP(y);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // ���ɣ��Ż������жϵķ�֦
                        if (pcCaptured <= JU_TO) {
                            // ��׽��(ʿ)����(��)��������迼��
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // ��׽������Ҫ�ж����Ƿ��ܱ���
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                else {
                                    // ��׽���˳�
                                    return pcCaptured;
                                }
                            }
                        }
                        else {
                            // ��׽�ڵ�������迼��
                            if (pcCaptured >= PAWN_FROM) {
                                // ��׽���˱�(��)��Ҫ�жϱ�(��)�Ƿ���Ӳ��ܱ���
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
            // ����ں����ƶ��ˣ����ж�������Ե�����
            lpsmv = FileMovePtr(x, y);
            for (i = 0; i < 2; i++) {
                sqDst = lpsmv->ucCannonCap[i] + FILE_DISP(x);
                if (sqDst != sqSrc) {
                    pcCaptured = this->ucpcSquares[sqDst];
                    if ((pcCaptured & nSideTag) != 0) {
                        pcCaptured -= nSideTag;
                        // ���ɣ��Ż������жϵķ�֦
                        if (pcCaptured <= JU_TO) {
                            // ��׽��(ʿ)����(��)��������迼��
                            if (pcCaptured >= H_FROM) {
                                if (pcCaptured <= H_TO) {
                                    // ��׽������Ҫ�ж����Ƿ��ܱ���
                                    if (!ifProtected(this->nowSide, sqDst)) {
                                        return pcCaptured;
                                    }
                                }
                                else {
                                    // ��׽���˳�
                                    return pcCaptured;
                                }
                            }
                        }
                        else {
                            // ��׽�ڵ�������迼��
                            if (pcCaptured >= PAWN_FROM) {
                                // ��׽���˱�(��)��Ҫ�жϱ�(��)�Ƿ���Ӳ��ܱ���
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