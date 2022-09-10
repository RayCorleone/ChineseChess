#include "define.h"
#include "position.h"
#include "movesort.h"

int HisTable[65536]; // 历史表

// 根据历史表对着法列表赋值
void myMoveSort::setHisTab(void) {
    int i, j, vl, nShift, nNewShift;
    nShift = 0;
    for (i = mvsIndex; i < mvsNum; i++) {
        // 如果着法的分值超过65536，对所有着法的分值作缩减使低于65536
        vl = HisTable[mvs[i].bmv] >> nShift;
        if (vl > 65535) {
            nNewShift = Bsr(vl) - 15;
            for (j = mvsIndex; j < i; j++) {
                mvs[j].wsc >>= nNewShift;
            }
            vl >>= nNewShift;
            nShift += nNewShift;
        }
        mvs[i].wsc = vl;
    }
}

// 排序
static const int cnShellStep[8] = {0, 1, 4, 13, 40, 121, 364, 1093};

void myMoveSort::mvsSort(void) {
    int i, j, nStep, nStepLevel;
    myMove mvsBest;
    nStepLevel = 1;
    while (cnShellStep[nStepLevel] < mvsNum - mvsIndex) {
        nStepLevel++;
    }
    nStepLevel--;
    while (nStepLevel > 0) {
        nStep = cnShellStep[nStepLevel];
        for (i = mvsIndex + nStep; i < mvsNum; i++) {
            mvsBest = mvs[i];
            j = i - nStep;
            while (j >= mvsIndex && mvsBest.wsc > mvs[j].wsc) {
                mvs[j + nStep] = mvs[j];
                j -= nStep;
            }
            mvs[j + nStep] = mvsBest;
        }
        nStepLevel--;
    }
}

int myMoveSort::initEvade(myPosition& pos, int mv, const uint16_t* lpwmvKiller) {
    int i, nLegal;
    staNum = STA_RE;
    mvsIndex = 0;
    mvsNum = pos.genAllMvs(mvs);
    setHisTab();
    nLegal = 0;
    for (i = mvsIndex; i < mvsNum; i++) {
        if (mvs[i].bmv == mv) {
            nLegal++;
            mvs[i].wsc = MAX_MVS;
        }
        else if (pos.doAMove(mvs[i].bmv)) {
            pos.unDoAMove();
            nLegal++;
            if (mvs[i].bmv == lpwmvKiller[0]) {
                mvs[i].wsc = MAX_MVS - 1;
            }
            else if (mvs[i].bmv == lpwmvKiller[1]) {
                mvs[i].wsc = MAX_MVS - 2;
            }
            else {
                mvs[i].wsc = min(mvs[i].wsc + 1, MAX_MVS - 3);
            }
        }
        else {
            mvs[i].wsc = 0;
        }
    }
    mvsSort();
    mvsNum = mvsIndex + nLegal;
    return (nLegal == 1 ? mvs[0].bmv : 0);
}

// 给出下一个即将搜索的着法
int myMoveSort::nextFS(const myPosition& pos) {
    switch (staNum) {
    case STA_HASH:
        staNum = STA_GEN_CAP;
        if (mvHash != 0) {
            return mvHash;
        }

      //生成所有吃子着法
    case STA_GEN_CAP:
        staNum = STA_WELLEAT;
        mvsIndex = 0;
        mvsNum = pos.genEatMvs(mvs);
        mvsSort();

        //MVV启发
    case STA_WELLEAT:
        if (mvsIndex < mvsNum && mvs[mvsIndex].wsc > 1) {
            mvsIndex++;
            return mvs[mvsIndex - 1].bmv;
        }

        // 杀手启发
    case STA_KA:
        staNum = STA_KB;
        if (mvKillerA != 0 && pos.ifLegalM(mvKillerA)) {
            return mvKillerA;
        }

        // 2
    case STA_KB:
        staNum = STA_NULL;
        if (mvKillerB != 0 && pos.ifLegalM(mvKillerB)) {
            return mvKillerB;
        }

        // 所有不吃子着法
    case STA_NULL:
        staNum = STA_RE;
        mvsNum += pos.genMvsWithoutEat(mvs + mvsNum);
        setHisTab();
        mvsSort();

        // 对剩余着法做历史表启发
    case STA_RE:
        if (mvsIndex < mvsNum) {
            mvsIndex++;
            return mvs[mvsIndex - 1].bmv;
        }
    default:
        return 0;
    }
}

// 生成根结点的着法
void myMoveSort::initRM(const myPosition& pos) {
    int i;
    mvsIndex = 0;
    mvsNum = pos.genAllMvs(mvs);
    for (i = 0; i < mvsNum; i++) {
        mvs[i].wsc = 1;
    }
    mvsSort();
}

// 更新根结点的着法排序列表
void myMoveSort::flashRM(int mv) {
    int i;
    for (i = 0; i < mvsNum; i++) {
        if (mvs[i].bmv == mv) {
            mvs[i].wsc = MAX_MVS;
        }
        else if (mvs[i].wsc > 0) {
            mvs[i].wsc--;
        }
    }
}
