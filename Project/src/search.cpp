#include <stdio.h>
#include "define.h"
#include "pregen.h"
#include "position.h"
#include "hash.h"
#include "ucci.h"
#include "book.h"
#include "movesort.h"
#include "search.h"

const int IID_DEPTH = 2;         // �ڲ�������������
const int UNCHANGED_DEPTH = 4;   // δ�ı�����ŷ������

const int DROPDOWN_VALUE = 20;   // ���ķ�ֵ
const int RESIGN_VALUE = 300;    // ����ķ�ֵ
const int DRAW_OFFER_VALUE = 40; // ��͵ķ�ֵ

mySearch Search;

// ������Ϣ���Ƿ�װ��ģ���ڲ���
static struct {
    int64_t llTime;                     // ��ʱ��
    bool bStop;                         // ��ֹ�źźͺ�̨˼����Ϊ����ֹ�ź�
    int nAllNodes;                      // �ܽ����
    int nMainNodes;                     // ���������Ľ����
    int nUnchanged;                     // δ�ı�����ŷ������
    uint16_t wmvPvLine[MAX_ROLL];   // ��Ҫ����·���ϵ��ŷ��б�
    uint16_t wmvKiller[MAX_DEP][2]; // ɱ���ŷ���
    myMoveSort MoveSort;            // �������ŷ�����
} Search2;

void BuildPos(myPosition& pos, const UCStruct& UcciComm) {
    int i, mv;
    pos.recoByFen(UcciComm.szFenStr);
    for (i = 0; i < UcciComm.mvsNum; i++) {
        mv = strToMv(UcciComm.lpdwMovesCoord[i]);
        if (mv == 0) {
            break;
        }
        if (pos.ifLegalM(mv) && pos.doAMove(mv) && pos.theCheckingM().CptDrw > 0) {
            // ʼ����pos.nMoveNum��ӳû���ӵĲ���
            pos.emptyDraw();
        }
    }
}

// �ж�����
inline void Interrupt() {
    if ((int)(getTime() - Search2.llTime) > Search.maxTime) {
        Search2.bStop = true;
    }
    return;
}

// �޺��ü�
static int HarmlessPruning(const myPosition& pos, int vlBeta) {
    int vl, vlRep;

    // ɱ�岽���ü���
    vl = pos.nDistance - BEST_SCORE;
    if (vl >= vlBeta) {
        return vl;
    }

    //����ü���
    if (pos.ifDraw()) {
        return 0; 
    }

    //  �ظ��ü���
    vlRep = pos.sameSitu();
    if (vlRep > 0) {
        return pos.sameSituScore(vlRep);
    }

    return -BEST_SCORE;
}

// �����;������ۺ���
inline int Evaluate(const myPosition& pos, int vlAlpha, int vlBeta) {
    int vl;
    vl = Search.ifKnow ? pos.Evaluate(vlAlpha, vlBeta) : pos.Material();
    return vl == pos.drawScore() ? vl - 1 : vl;
}

//�������ж�|��̬��������
static int SearchQuiesc(myPosition& pos, int vlAlpha, int vlBeta) {
    int vlBest, vl, mv;
    bool bInCheck;
    myMoveSort MoveSort;
    // ��̬�������̰������¼������裺
    Search2.nAllNodes++;

    // �޺��ü���
    vl = HarmlessPruning(pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    // �ﵽ������ȣ�ֱ�ӷ�������ֵ��
    if (pos.nDistance == MAX_DEP) {
        return Evaluate(pos, vlAlpha, vlBeta);
    }

    //��ʼ��
    vlBest = -BEST_SCORE;
    bInCheck = (pos.theCheckingM().ChkChs > 0);

    //���ڱ������ľ��棬����ȫ���ŷ���
    if (bInCheck) {
        MoveSort.genAll(pos);
    }
    else {
        //����δ�������ľ��棬�������ŷ�ǰ���ȳ��Կ���(��������)�����Ծ��������ۣ�
        vl = Evaluate(pos, vlAlpha, vlBeta);
        if (vl >= vlBeta) {
            return vl;
        }
        vlBest = vl;
        vlAlpha = max(vl, vlAlpha);

        // ����δ�������ľ��棬���ɲ��������г����ŷ�(MVV(LVA)����)��
        MoveSort.initQ(pos);
    }

    // ��Alpha-Beta�㷨������Щ�ŷ���
    while ((mv = MoveSort.getNextQ(bInCheck)) != 0) {
        if (pos.doAMove(mv)) {
            vl = -SearchQuiesc(pos, -vlBeta, -vlAlpha);
            pos.unDoAMove();
            if (vl > vlBest) {
                if (vl >= vlBeta) {
                    return vl;
                }
                vlBest = vl;
                vlAlpha = max(vl, vlAlpha);
            }
        }
    }

    // ���ط�ֵ��
    if (vlBest == -BEST_SCORE) {
        return pos.nDistance - BEST_SCORE;
    }
    else {
        return vlBest;
    }
}

const bool NO_NULL = true;

//��Ҫ�ж�|�㴰����ȫ��������
static int SearchCut(int vlBeta, int nDepth, bool bNoNull = false) {
    int nNewDepth, vlBest, vl;
    int mvHash, mv, mvEvade;
    myMoveSort MoveSort;
//��Ҷ�ӽ�㴦���þ�̬������
    if (nDepth <= 0) {
        return SearchQuiesc(Search.pos, vlBeta - 1, vlBeta);
    }
    Search2.nAllNodes++;

    // �޺��ü���
    vl = HarmlessPruning(Search.pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    // �û��ü���
    vl = getHT(Search.pos, vlBeta - 1, vlBeta, nDepth, bNoNull, mvHash);
    if (Search.ifHT && vl > -BEST_SCORE) {
        return vl;
    }

    // �ﵽ������ȣ�ֱ�ӷ�������ֵ��
    if (Search.pos.nDistance == MAX_DEP) {
        return Evaluate(Search.pos, vlBeta - 1, vlBeta);
    }

    Search2.nMainNodes++;
    vlBest = -BEST_SCORE;

    // ���Կ��Ųü���
    if (Search.ifNM && !bNoNull && Search.pos.theCheckingM().ChkChs <= 0 && Search.pos.ifAblNul()) {
        Search.pos.nullMove();
        vl = -SearchCut(1 - vlBeta, nDepth - HT_NULL_DEP - 1, NO_NULL);
        Search.pos.unDoNullMove();

        Interrupt();
        if (Search2.bStop) {
            return vlBest;
        }

        if (vl >= vlBeta) {
            if (Search.pos.nullCanPass()) {
                // ������Ųü��������飬��ô��¼�������Ϊ(HT_NULL_DEP + 1)��
                memHT(Search.pos, HT_BET, vl, max(nDepth, HT_NULL_DEP + 1), 0);
                return vl;
            }
            else if (SearchCut(vlBeta, nDepth - HT_NULL_DEP, NO_NULL) >= vlBeta) {
                //  ������Ųü������飬��ô��¼�������Ϊ(HT_NULL_DEP)��
                memHT(Search.pos, HT_BET, vl, max(nDepth, HT_NULL_DEP), 0);
                return vl;
            }
        }
    }

    // ��ʼ����
    if (Search.pos.theCheckingM().ChkChs > 0) {
        // ����ǽ������棬��ô��������Ӧ���ŷ���
        mvEvade = MoveSort.initEvade(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
    }
    else {
        // ������ǽ������棬��ôʹ���������ŷ��б�
        MoveSort.initFS(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
        mvEvade = 0;
    }

    // ����"myMoveSort::nextFS()"���̵��ŷ�˳����һ������
    while ((mv = MoveSort.nextFS(Search.pos)) != 0) {
        if (Search.pos.doAMove(mv)) {

            //����ѡ�������죻
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 || mvEvade != 0 ? nDepth : nDepth - 1);

            // �㴰��������
            vl = -SearchCut(1 - vlBeta, nNewDepth);
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop) {
                return vlBest;
            }

            // �ض��ж���
            if (vl > vlBest) {
                vlBest = vl;
                if (vl >= vlBeta) {
                    memHT(Search.pos, HT_BET, vlBest, nDepth, mv);
                    if (!MoveSort.wellEat(Search.pos, mv)) {
                        setBMv(mv, nDepth, Search2.wmvKiller[Search.pos.nDistance]);
                    }
                    return vlBest;
                }
            }
        }
    }

    // ���ضϴ�ʩ��
    if (vlBest == -BEST_SCORE) {
        return Search.pos.nDistance - BEST_SCORE;
    }
    else {
        memHT(Search.pos, HT_ALP, vlBest, nDepth, mvEvade);
        return vlBest;
    }
}

// ������Ҫ����
static void AppendPvLine(uint16_t* lpwmvDst, uint16_t mv, const uint16_t* lpwmvSrc) {
    *lpwmvDst = mv;
    lpwmvDst++;
    while (*lpwmvSrc != 0) {
        *lpwmvDst = *lpwmvSrc;
        lpwmvSrc++;
        lpwmvDst++;
    }
    *lpwmvDst = 0;
}

static int SearchPV(int vlAlpha, int vlBeta, int nDepth, uint16_t* lpwmvPvLine) {
    int nNewDepth, nHashFlag, vlBest, vl;
    int mvBest, mvHash, mv, mvEvade;
    myMoveSort MoveSort;
    uint16_t wmvPvLine[MAX_DEP];
    // ��ȫ�������̰������¼������裺

    // Ҷ�ӽ�㴦���þ�̬������
    *lpwmvPvLine = 0;
    if (nDepth <= 0) {
        return SearchQuiesc(Search.pos, vlAlpha, vlBeta);
    }
    Search2.nAllNodes++;

    // �޺��ü���
    vl = HarmlessPruning(Search.pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    //�û��ü���
    vl = getHT(Search.pos, vlAlpha, vlBeta, nDepth, NO_NULL, mvHash);
    if (Search.ifHT && vl > -BEST_SCORE) {
        // ����PV��㲻�����û��ü������Բ��ᷢ��PV·���жϵ����
        return vl;
    }
//�ﵽ������ȣ�ֱ�ӷ�������ֵ��
    if (Search.pos.nDistance == MAX_DEP) {
        return Evaluate(Search.pos, vlAlpha, vlBeta);
    }

    Search2.nMainNodes++;
    vlBest = -BEST_SCORE;

    // �ڲ���������������
    if (nDepth > IID_DEPTH && mvHash == 0) {
        vl = SearchPV(vlAlpha, vlBeta, nDepth / 2, wmvPvLine);
        if (vl <= vlAlpha) {
            vl = SearchPV(-BEST_SCORE, vlBeta, nDepth / 2, wmvPvLine);
        }

        Interrupt();
        if (Search2.bStop) {
            return vlBest;
        }
        mvHash = wmvPvLine[0];
    }

    //��ʼ����
    mvBest = 0;
    nHashFlag = HT_ALP;
    if (Search.pos.theCheckingM().ChkChs > 0) {
        // ����ǽ������棬��ô��������Ӧ���ŷ���
        mvEvade = MoveSort.initEvade(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
    }
    else {
        // ������ǽ������棬��ôʹ���������ŷ��б�
        MoveSort.initFS(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
        mvEvade = 0;
    }

    // ����"myMoveSort::nextFS()"���̵��ŷ�˳����һ������
    while ((mv = MoveSort.nextFS(Search.pos)) != 0) {
        if (Search.pos.doAMove(mv)) {

            // ����ѡ�������죻
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 || mvEvade != 0 ? nDepth : nDepth - 1);

            // ��Ҫ����������
            if (vlBest == -BEST_SCORE) {
                vl = -SearchPV(-vlBeta, -vlAlpha, nNewDepth, wmvPvLine);
            }
            else {
                vl = -SearchCut(-vlAlpha, nNewDepth);
                if (vl > vlAlpha && vl < vlBeta) {
                    vl = -SearchPV(-vlBeta, -vlAlpha, nNewDepth, wmvPvLine);
                }
            }
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop) {
                return vlBest;
            }

            // Alpha-Beta�߽��ж���
            if (vl > vlBest) {
                vlBest = vl;
                if (vl >= vlBeta) {
                    mvBest = mv;
                    nHashFlag = HT_BET;
                    break;
                }
                if (vl > vlAlpha) {
                    vlAlpha = vl;
                    mvBest = mv;
                    nHashFlag = HT_PV;
                    AppendPvLine(lpwmvPvLine, mv, wmvPvLine);
                }
            }
        }
    }

    // �����û�����ʷ���ɱ���ŷ���
    if (vlBest == -BEST_SCORE) {
        return Search.pos.nDistance - BEST_SCORE;
    }
    else {
        memHT(Search.pos, nHashFlag, vlBest, nDepth, mvEvade == 0 ? mvBest : mvEvade);
        if (mvBest != 0 && !MoveSort.wellEat(Search.pos, mvBest)) {
            setBMv(mvBest, nDepth, Search2.wmvKiller[Search.pos.nDistance]);
        }
        return vlBest;
    }
}

static int SearchRoot(int nDepth) {
    int nNewDepth, vlBest, vl, mv, nCurrMove;
    uint16_t wmvPvLine[MAX_DEP];
    // ������������̰������¼������裺

    // ��ʼ��
    vlBest = -BEST_SCORE;
    Search2.MoveSort.resetRM();

    // ��һ����ÿ���ŷ�(Ҫ���˽�ֹ�ŷ�)
    nCurrMove = 0;
    while ((mv = Search2.MoveSort.nextRM()) != 0) {
        if (Search.pos.doAMove(mv)) {

            // ����ѡ��������(ֻ���ǽ�������)
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 ? nDepth : nDepth - 1);

            // ��Ҫ��������
            if (vlBest == -BEST_SCORE) {
                vl = -SearchPV(-BEST_SCORE, BEST_SCORE, nNewDepth, wmvPvLine);
            }
            else {
                vl = -SearchCut(-vlBest, nNewDepth);
                if (vl > vlBest) { // ���ﲻ��Ҫ" && vl < BEST_SCORE"��
                    vl = -SearchPV(-BEST_SCORE, -vlBest, nNewDepth, wmvPvLine);
                }
            }
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop) {
                return vlBest;
            }

            //  Alpha-Beta�߽��ж�("vlBest"������"SearchPV()"�е�"vlAlpha")
            if (vl > vlBest) {

                //  �����������һ�ŷ�����ô"δ�ı�����ŷ�"�ļ�������1����������
                Search2.nUnchanged = (vlBest == -BEST_SCORE ? Search2.nUnchanged + 1 : 0);
                vlBest = vl;

                //  ����������ŷ�ʱ��¼��Ҫ����
                AppendPvLine(Search2.wmvPvLine, mv, wmvPvLine);

                //  ���Ҫ��������ԣ���AlphaֵҪ���������������������ɱ��ʱ�����������
                if (vlBest > -KILL_SCORE && vlBest < KILL_SCORE) {
                    vlBest += (Search.randomNum.NextLong() & Search.randMNum) -
                        (Search.randomNum.NextLong() & Search.randMNum);
                    vlBest = (vlBest == Search.pos.drawScore() ? vlBest - 1 : vlBest);
                }

                // ���¸�����ŷ��б�
                Search2.MoveSort.flashRM(mv);
            }
        }
    }
    return vlBest;
}

static bool SearchUnique(int vlBeta, int nDepth) {
    int vl, mv;
    Search2.MoveSort.resetRM(ROOT_UNI);
    // ������һ���ŷ�
    while ((mv = Search2.MoveSort.nextRM()) != 0) {
        if (Search.pos.doAMove(mv)) {
            vl = -SearchCut(1 - vlBeta, Search.pos.theCheckingM().ChkChs > 0 ? nDepth : nDepth - 1);
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop || vl >= vlBeta) {
                return false;
            }
        }
    }
    return true;
}

// ����������
void SearchMain(int nDepth) {
    Search2.llTime = getTime();

    int i, vl, vlLast;
    int nCurrTimer, nLimitTimer;
    bool bUnique;
    int nBookMoves;
    uint32_t dwMoveStr;
    myBook bks[MAX_MOVS];
    // ���������̰������¼������裺

    // ����������ֱ�ӷ���
    if (Search.pos.ifDraw() || Search.pos.sameSitu(3) > 0) {
        printf("nobestmove\n");
        fflush(stdout);
        return;
    }

    // �ӿ��ֿ��������ŷ�
    if (Search.ifBook) {
        // a. ��ȡ���ֿ��е������߷�
        nBookMoves = getBookMvs(Search.pos, bks);
        if (nBookMoves > 0) {
            vl = 0;
            for (i = 0; i < nBookMoves; i++) {
                vl += bks[i].wsc;
            }
            // b. ����Ȩ�����ѡ��һ���߷�
            vl = Search.randomNum.NextLong() % (uint32_t)vl;
            for (i = 0; i < nBookMoves; i++) {
                vl -= bks[i].wsc;
                if (vl < 0) {
                    break;
                }
            }
            // c. ������ֿ��е��ŷ�����ѭ�����棬��ô��������ŷ�
            Search.pos.doAMove(bks[i].bmv);
            if (Search.pos.sameSitu(3) == 0) {
                dwMoveStr = mvToStr(bks[i].bmv);
                printf("bestmove %.4s", (const char*)&dwMoveStr);
                // d. ������̨˼�����ŷ�(���ֿ��е�һ����Ȩ�����ĺ����ŷ�)
                nBookMoves = getBookMvs(Search.pos, bks);
                Search.pos.unDoAMove();
                printf("\n");
                fflush(stdout);
                return;
            }
            Search.pos.unDoAMove();
        }
    }

    //  ���ɸ�����ÿ���ŷ�
    Search2.MoveSort.initRM(Search.pos);

    //��ʼ��ʱ��ͼ�����
    Search2.bStop = false;
    Search2.nAllNodes = Search2.nMainNodes = Search2.nUnchanged = 0;
    Search2.wmvPvLine[0] = 0;
    emptyKillTab(Search2.wmvKiller);
    emptyHisTab();
    emptyHT();
    vlLast = 0;
    bUnique = false;
    nCurrTimer = 0;

    // ��������������
    for (i = 1; i <= nDepth; i++) {
        printf("���������� ��%d�㣡\n", i);

        // ���������
        vl = SearchRoot(i);
        if (Search2.bStop) {
            if (vl > -BEST_SCORE) {
                vlLast = vl; // ������vlLast�������ж������Ͷ����������Ҫ�������һ��ֵ
            }
            break; // û����������"vl"�ǿɿ�ֵ
        }

        // �������ʱ�䳬���ʵ�ʱ�ޣ�����ֹ����
        nLimitTimer = Search.maxTime;
        // a. ���û��ʹ�ÿ��Ųü�����ô�ʵ�ʱ�޼���(��Ϊ��֦���Ӽӱ���)
        nLimitTimer = (Search.ifNM ? nLimitTimer : nLimitTimer / 2);
        // b. �����ǰ����ֵû�����ǰһ��ܶ࣬��ô�ʵ�ʱ�޼���
        nLimitTimer = (vl + DROPDOWN_VALUE >= vlLast ? nLimitTimer / 2 : nLimitTimer);
        // c. �������ŷ��������û�б仯����ô�ʵ�ʱ�޼���
        nLimitTimer = (Search2.nUnchanged >= UNCHANGED_DEPTH ? nLimitTimer / 2 : nLimitTimer);
        nCurrTimer = (int)(getTime() - Search2.llTime);
        if (nCurrTimer > nLimitTimer) {
            vlLast = vl;
            break; // �����Ƿ�������"vlLast"���Ѹ���
        }
        vlLast = vl;

        // ������ɱ������ֹ����
        if (vlLast > KILL_SCORE || vlLast < -KILL_SCORE) {
            break;
        }

        //  ��Ψһ�ŷ�������ֹ����
        if (SearchUnique(1 - KILL_SCORE, i)) {
            bUnique = true;
            break;
        }
    }

    // �������ŷ�
    if (Search2.wmvPvLine[0] != 0) {
        dwMoveStr = mvToStr(Search2.wmvPvLine[0]);
        printf("bestmove %.4s", (const char*)&dwMoveStr);
    }
    else {
        printf("nobestmove");
    }
    printf("\n");
    fflush(stdout);
}