#include <stdio.h>
#include "define.h"
#include "position.h"
#include "hash.h"

int tabSize;
myHashTable *tabP;

// 存储哈希置换表局面信息
void memHT(const myPosition& pos, int flag, int score, int dep, int mv) {
    myHashTable ha;
    int i, nHashDepth, nMinDepth, nMinLayer;
    //根据score调整杀招步数
    if (score > KILL_SCORE) {
        if (mv == 0 && score <= LONG_LOSE_SCORE)  return; //长将
        score += pos.nDistance;
    }
    else if (score < -KILL_SCORE) {
        if (mv == 0 && score >= -LONG_LOSE_SCORE) return; 
        score -= pos.nDistance;
    }
    else if (score == pos.drawScore() && mv == 0) return;

    // 深入
    nMinDepth = 512;
    nMinLayer = 0;
    for (i = 0; i < HT_DEP; i++) {
        ha = getItem(pos, i);
        // 匹配相同situ，更新
        if (ifZLockEqu(ha, pos)) {
            // 更新
            if ((flag & HT_ALP) != 0 && (ha.alpDep <= dep || ha.alpScore >= score)) {
                ha.alpDep = dep;
                ha.alpScore = score;
            }
            if ((flag & HT_BET) != 0 && (ha.betDep <= dep || ha.betScore <= score) && (mv != 0 || ha.bmv == 0)) {
                ha.betDep = dep;
                ha.betScore = score;
            }
            if (mv != 0)  ha.bmv = mv;
            getItem(pos, i) = ha;
            return;
        }
        // 无匹配，返回深度最小的置换表项；
        nHashDepth = max((ha.alpDep == 0 ? 0 : ha.alpDep + 256),
            (ha.bmv == 0 ? ha.betDep : ha.betDep + 256));
        if (nHashDepth < nMinDepth) {
            nMinDepth = nHashDepth;
            nMinLayer = i;
        }
    }
    //存HT
    ha.dwZLockA = pos.zobr.dwLock0;
    ha.dwZLockB = pos.zobr.dwLock1;
    ha.bmv = mv;
    ha.alpDep = ha.betDep = 0;
    ha.alpScore = ha.betScore = 0;
    if ((flag & HT_ALP) != 0) {
        ha.alpDep = dep;
        ha.alpScore = score;
    }
    if ((flag & HT_BET) != 0) {
        ha.betDep = dep;
        ha.betScore = score;
    }
    getItem(pos, nMinLayer) = ha;
}

inline int scoreDepend(const myPosition& pos, bool& bBanNode, bool& bMateNode, int score) {
    bBanNode = bMateNode = false;
    if (score > KILL_SCORE) {
        if (score <= LONG_LOSE_SCORE) bBanNode = true;
        else {
            bMateNode = true;
            score -= pos.nDistance;
        }
    }
    else if (score < -KILL_SCORE) {
        if (score >= -LONG_LOSE_SCORE) bBanNode = true;
        else {
            bMateNode = true;
            score += pos.nDistance;
        }
    }
    else if (score == pos.drawScore()) bBanNode = true;
    return score;
}

inline bool ifSteaty(myPosition& pos, int mv) {
    if (mv == 0) return true;//无后续着法
    if (pos.ucpcSquares[getDir(mv)] != 0) return true;//稳定吃子
    if (!pos.doAMove(mv)) return true;
    return false;
}

static bool ifSt2(const myPosition& pos, int mv) {
    myHashTable ha;
    int i, mvsNum;
    bool bStable;
    myPosition& posMutable = (myPosition&)pos;
    mvsNum = 0;
    bStable = true;
    while (!ifSteaty(posMutable, mv)) {
        mvsNum++; 
        if (posMutable.sameSitu() > 0) {
            bStable = false;
            break;
        }
        for (i = 0; i < HT_DEP; i++) {
            ha = getItem(posMutable, i);
            if (ifZLockEqu(ha, posMutable)) break;
        }
        mv = (i == HT_DEP ? 0 : ha.bmv);
    }
    for (i = 0; i < mvsNum; i++) {
        posMutable.unDoAMove();
    }
    return bStable;
}
// 获取置换表局面信息
int getHT(const myPosition& pos, int alpScore, int betScore, int dep, bool bNoNull, int& mv) {
    myHashTable hsh;
    int i, score;
    bool bBanNode, bMateNode;
    // 逐层获取表项
    mv = 0;
    for (i = 0; i < HT_DEP; i++) {
        hsh = getItem(pos, i);
        if (ifZLockEqu(hsh, pos)) {
            mv = hsh.bmv;
            break;
        }
    }
    if (i == HT_DEP) {
        return -BEST_SCORE;
    }

    // 匹配Beta边界？
    if (hsh.betDep > 0) {
        score = scoreDepend(pos, bBanNode, bMateNode, hsh.betScore);
        if (!bBanNode && !(hsh.bmv == 0 && bNoNull) && (hsh.betDep >= dep || bMateNode) && score >= betScore) {
            if (hsh.bmv == 0 || ifSt2(pos, hsh.bmv)) return score;
        }
    }

    // 匹配alp边界？
    if (hsh.alpDep > 0) {
        score = scoreDepend(pos, bBanNode, bMateNode, hsh.alpScore);
        if (!bBanNode && (hsh.alpDep >= dep || bMateNode) && score <= alpScore) {
            if (hsh.bmv == 0 || ifSt2(pos, hsh.bmv)) return score;
        }
    }
    return -BEST_SCORE;
}

