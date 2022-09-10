#include <stdio.h>
#include "define.h"
#include "pregen.h"
#include "position.h"
#include "hash.h"
#include "ucci.h"
#include "book.h"
#include "movesort.h"
#include "search.h"

const int IID_DEPTH = 2;         // 内部迭代加深的深度
const int UNCHANGED_DEPTH = 4;   // 未改变最佳着法的深度

const int DROPDOWN_VALUE = 20;   // 落后的分值
const int RESIGN_VALUE = 300;    // 认输的分值
const int DRAW_OFFER_VALUE = 40; // 提和的分值

mySearch Search;

// 搜索信息，是封装在模块内部的
static struct {
    int64_t llTime;                     // 计时器
    bool bStop;                         // 中止信号和后台思考认为的中止信号
    int nAllNodes;                      // 总结点数
    int nMainNodes;                     // 主搜索树的结点数
    int nUnchanged;                     // 未改变最佳着法的深度
    uint16_t wmvPvLine[MAX_ROLL];   // 主要变例路线上的着法列表
    uint16_t wmvKiller[MAX_DEP][2]; // 杀手着法表
    myMoveSort MoveSort;            // 根结点的着法序列
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
            // 始终让pos.nMoveNum反映没吃子的步数
            pos.emptyDraw();
        }
    }
}

// 中断搜索
inline void Interrupt() {
    if ((int)(getTime() - Search2.llTime) > Search.maxTime) {
        Search2.bStop = true;
    }
    return;
}

// 无害裁剪
static int HarmlessPruning(const myPosition& pos, int vlBeta) {
    int vl, vlRep;

    // 杀棋步数裁剪；
    vl = pos.nDistance - BEST_SCORE;
    if (vl >= vlBeta) {
        return vl;
    }

    //和棋裁剪；
    if (pos.ifDraw()) {
        return 0; 
    }

    //  重复裁剪；
    vlRep = pos.sameSitu();
    if (vlRep > 0) {
        return pos.sameSituScore(vlRep);
    }

    return -BEST_SCORE;
}

// 调整型局面评价函数
inline int Evaluate(const myPosition& pos, int vlAlpha, int vlBeta) {
    int vl;
    vl = Search.ifKnow ? pos.Evaluate(vlAlpha, vlBeta) : pos.Material();
    return vl == pos.drawScore() ? vl - 1 : vl;
}

//·无需中断|静态搜索例程
static int SearchQuiesc(myPosition& pos, int vlAlpha, int vlBeta) {
    int vlBest, vl, mv;
    bool bInCheck;
    myMoveSort MoveSort;
    // 静态搜索例程包括以下几个步骤：
    Search2.nAllNodes++;

    // 无害裁剪；
    vl = HarmlessPruning(pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    // 达到极限深度，直接返回评价值；
    if (pos.nDistance == MAX_DEP) {
        return Evaluate(pos, vlAlpha, vlBeta);
    }

    //初始化
    vlBest = -BEST_SCORE;
    bInCheck = (pos.theCheckingM().ChkChs > 0);

    //对于被将军的局面，生成全部着法；
    if (bInCheck) {
        MoveSort.genAll(pos);
    }
    else {
        //对于未被将军的局面，在生成着法前首先尝试空着(空着启发)，即对局面作评价；
        vl = Evaluate(pos, vlAlpha, vlBeta);
        if (vl >= vlBeta) {
            return vl;
        }
        vlBest = vl;
        vlAlpha = max(vl, vlAlpha);

        // 对于未被将军的局面，生成并排序所有吃子着法(MVV(LVA)启发)；
        MoveSort.initQ(pos);
    }

    // 用Alpha-Beta算法搜索这些着法；
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

    // 返回分值。
    if (vlBest == -BEST_SCORE) {
        return pos.nDistance - BEST_SCORE;
    }
    else {
        return vlBest;
    }
}

const bool NO_NULL = true;

//需要中断|零窗口完全搜索例程
static int SearchCut(int vlBeta, int nDepth, bool bNoNull = false) {
    int nNewDepth, vlBest, vl;
    int mvHash, mv, mvEvade;
    myMoveSort MoveSort;
//在叶子结点处调用静态搜索；
    if (nDepth <= 0) {
        return SearchQuiesc(Search.pos, vlBeta - 1, vlBeta);
    }
    Search2.nAllNodes++;

    // 无害裁剪；
    vl = HarmlessPruning(Search.pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    // 置换裁剪；
    vl = getHT(Search.pos, vlBeta - 1, vlBeta, nDepth, bNoNull, mvHash);
    if (Search.ifHT && vl > -BEST_SCORE) {
        return vl;
    }

    // 达到极限深度，直接返回评价值；
    if (Search.pos.nDistance == MAX_DEP) {
        return Evaluate(Search.pos, vlBeta - 1, vlBeta);
    }

    Search2.nMainNodes++;
    vlBest = -BEST_SCORE;

    // 尝试空着裁剪；
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
                // 如果空着裁剪不带检验，那么记录深度至少为(HT_NULL_DEP + 1)；
                memHT(Search.pos, HT_BET, vl, max(nDepth, HT_NULL_DEP + 1), 0);
                return vl;
            }
            else if (SearchCut(vlBeta, nDepth - HT_NULL_DEP, NO_NULL) >= vlBeta) {
                //  如果空着裁剪带检验，那么记录深度至少为(HT_NULL_DEP)；
                memHT(Search.pos, HT_BET, vl, max(nDepth, HT_NULL_DEP), 0);
                return vl;
            }
        }
    }

    // 初始化；
    if (Search.pos.theCheckingM().ChkChs > 0) {
        // 如果是将军局面，那么生成所有应将着法；
        mvEvade = MoveSort.initEvade(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
    }
    else {
        // 如果不是将军局面，那么使用正常的着法列表。
        MoveSort.initFS(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
        mvEvade = 0;
    }

    // 按照"myMoveSort::nextFS()"例程的着法顺序逐一搜索；
    while ((mv = MoveSort.nextFS(Search.pos)) != 0) {
        if (Search.pos.doAMove(mv)) {

            //尝试选择性延伸；
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 || mvEvade != 0 ? nDepth : nDepth - 1);

            // 零窗口搜索；
            vl = -SearchCut(1 - vlBeta, nNewDepth);
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop) {
                return vlBest;
            }

            // 截断判定；
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

    // 不截断措施。
    if (vlBest == -BEST_SCORE) {
        return Search.pos.nDistance - BEST_SCORE;
    }
    else {
        memHT(Search.pos, HT_ALP, vlBest, nDepth, mvEvade);
        return vlBest;
    }
}

// 连接主要变例
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
    // 完全搜索例程包括以下几个步骤：

    // 叶子结点处调用静态搜索；
    *lpwmvPvLine = 0;
    if (nDepth <= 0) {
        return SearchQuiesc(Search.pos, vlAlpha, vlBeta);
    }
    Search2.nAllNodes++;

    // 无害裁剪；
    vl = HarmlessPruning(Search.pos, vlBeta);
    if (vl > -BEST_SCORE) {
        return vl;
    }

    //置换裁剪；
    vl = getHT(Search.pos, vlAlpha, vlBeta, nDepth, NO_NULL, mvHash);
    if (Search.ifHT && vl > -BEST_SCORE) {
        // 由于PV结点不适用置换裁剪，所以不会发生PV路线中断的情况
        return vl;
    }
//达到极限深度，直接返回评价值；
    if (Search.pos.nDistance == MAX_DEP) {
        return Evaluate(Search.pos, vlAlpha, vlBeta);
    }

    Search2.nMainNodes++;
    vlBest = -BEST_SCORE;

    // 内部迭代加深启发；
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

    //初始化；
    mvBest = 0;
    nHashFlag = HT_ALP;
    if (Search.pos.theCheckingM().ChkChs > 0) {
        // 如果是将军局面，那么生成所有应将着法；
        mvEvade = MoveSort.initEvade(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
    }
    else {
        // 如果不是将军局面，那么使用正常的着法列表。
        MoveSort.initFS(Search.pos, mvHash, Search2.wmvKiller[Search.pos.nDistance]);
        mvEvade = 0;
    }

    // 按照"myMoveSort::nextFS()"例程的着法顺序逐一搜索；
    while ((mv = MoveSort.nextFS(Search.pos)) != 0) {
        if (Search.pos.doAMove(mv)) {

            // 尝试选择性延伸；
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 || mvEvade != 0 ? nDepth : nDepth - 1);

            // 主要变例搜索；
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

            // Alpha-Beta边界判定；
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

    // 更新置换表、历史表和杀手着法表。
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
    // 根结点搜索例程包括以下几个步骤：

    // 初始化
    vlBest = -BEST_SCORE;
    Search2.MoveSort.resetRM();

    // 逐一搜索每个着法(要过滤禁止着法)
    nCurrMove = 0;
    while ((mv = Search2.MoveSort.nextRM()) != 0) {
        if (Search.pos.doAMove(mv)) {

            // 尝试选择性延伸(只考虑将军延伸)
            nNewDepth = (Search.pos.theCheckingM().ChkChs > 0 ? nDepth : nDepth - 1);

            // 主要变例搜索
            if (vlBest == -BEST_SCORE) {
                vl = -SearchPV(-BEST_SCORE, BEST_SCORE, nNewDepth, wmvPvLine);
            }
            else {
                vl = -SearchCut(-vlBest, nNewDepth);
                if (vl > vlBest) { // 这里不需要" && vl < BEST_SCORE"了
                    vl = -SearchPV(-BEST_SCORE, -vlBest, nNewDepth, wmvPvLine);
                }
            }
            Search.pos.unDoAMove();

            Interrupt();
            if (Search2.bStop) {
                return vlBest;
            }

            //  Alpha-Beta边界判定("vlBest"代替了"SearchPV()"中的"vlAlpha")
            if (vl > vlBest) {

                //  如果搜索到第一着法，那么"未改变最佳着法"的计数器加1，否则清零
                Search2.nUnchanged = (vlBest == -BEST_SCORE ? Search2.nUnchanged + 1 : 0);
                vlBest = vl;

                //  搜索到最佳着法时记录主要变例
                AppendPvLine(Search2.wmvPvLine, mv, wmvPvLine);

                //  如果要考虑随机性，则Alpha值要作随机浮动，但已搜索到杀棋时不作随机浮动
                if (vlBest > -KILL_SCORE && vlBest < KILL_SCORE) {
                    vlBest += (Search.randomNum.NextLong() & Search.randMNum) -
                        (Search.randomNum.NextLong() & Search.randMNum);
                    vlBest = (vlBest == Search.pos.drawScore() ? vlBest - 1 : vlBest);
                }

                // 更新根结点着法列表
                Search2.MoveSort.flashRM(mv);
            }
        }
    }
    return vlBest;
}

static bool SearchUnique(int vlBeta, int nDepth) {
    int vl, mv;
    Search2.MoveSort.resetRM(ROOT_UNI);
    // 跳过第一个着法
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

// 主搜索例程
void SearchMain(int nDepth) {
    Search2.llTime = getTime();

    int i, vl, vlLast;
    int nCurrTimer, nLimitTimer;
    bool bUnique;
    int nBookMoves;
    uint32_t dwMoveStr;
    myBook bks[MAX_MOVS];
    // 主搜索例程包括以下几个步骤：

    // 遇到和棋则直接返回
    if (Search.pos.ifDraw() || Search.pos.sameSitu(3) > 0) {
        printf("nobestmove\n");
        fflush(stdout);
        return;
    }

    // 从开局库中搜索着法
    if (Search.ifBook) {
        // a. 获取开局库中的所有走法
        nBookMoves = getBookMvs(Search.pos, bks);
        if (nBookMoves > 0) {
            vl = 0;
            for (i = 0; i < nBookMoves; i++) {
                vl += bks[i].wsc;
            }
            // b. 根据权重随机选择一个走法
            vl = Search.randomNum.NextLong() % (uint32_t)vl;
            for (i = 0; i < nBookMoves; i++) {
                vl -= bks[i].wsc;
                if (vl < 0) {
                    break;
                }
            }
            // c. 如果开局库中的着法够成循环局面，那么不走这个着法
            Search.pos.doAMove(bks[i].bmv);
            if (Search.pos.sameSitu(3) == 0) {
                dwMoveStr = mvToStr(bks[i].bmv);
                printf("bestmove %.4s", (const char*)&dwMoveStr);
                // d. 给出后台思考的着法(开局库中第一个即权重最大的后续着法)
                nBookMoves = getBookMvs(Search.pos, bks);
                Search.pos.unDoAMove();
                printf("\n");
                fflush(stdout);
                return;
            }
            Search.pos.unDoAMove();
        }
    }

    //  生成根结点的每个着法
    Search2.MoveSort.initRM(Search.pos);

    //初始化时间和计数器
    Search2.bStop = false;
    Search2.nAllNodes = Search2.nMainNodes = Search2.nUnchanged = 0;
    Search2.wmvPvLine[0] = 0;
    emptyKillTab(Search2.wmvKiller);
    emptyHisTab();
    emptyHT();
    vlLast = 0;
    bUnique = false;
    nCurrTimer = 0;

    // 做迭代加深搜索
    for (i = 1; i <= nDepth; i++) {
        printf("·迭代加深 第%d层！\n", i);

        // 搜索根结点
        vl = SearchRoot(i);
        if (Search2.bStop) {
            if (vl > -BEST_SCORE) {
                vlLast = vl; // 跳出后，vlLast会用来判断认输或投降，所以需要给定最近一个值
            }
            break; // 没有跳出，则"vl"是可靠值
        }

        // 如果搜索时间超过适当时限，则终止搜索
        nLimitTimer = Search.maxTime;
        // a. 如果没有使用空着裁剪，那么适当时限减半(因为分枝因子加倍了)
        nLimitTimer = (Search.ifNM ? nLimitTimer : nLimitTimer / 2);
        // b. 如果当前搜索值没有落后前一层很多，那么适当时限减半
        nLimitTimer = (vl + DROPDOWN_VALUE >= vlLast ? nLimitTimer / 2 : nLimitTimer);
        // c. 如果最佳着法连续多层没有变化，那么适当时限减半
        nLimitTimer = (Search2.nUnchanged >= UNCHANGED_DEPTH ? nLimitTimer / 2 : nLimitTimer);
        nCurrTimer = (int)(getTime() - Search2.llTime);
        if (nCurrTimer > nLimitTimer) {
            vlLast = vl;
            break; // 不管是否跳出，"vlLast"都已更新
        }
        vlLast = vl;

        // 搜索到杀棋则终止搜索
        if (vlLast > KILL_SCORE || vlLast < -KILL_SCORE) {
            break;
        }

        //  是唯一着法，则终止搜索
        if (SearchUnique(1 - KILL_SCORE, i)) {
            bUnique = true;
            break;
        }
    }

    // 输出最佳着法
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