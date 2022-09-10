#ifndef MOVESORT_H
#define MOVESORT_H

#include <string.h>
#include "define.h"
#include "position.h"

const int MAX_DEP = 64;       
const int MAX_MVS = 65535; 
extern int HisTable[65536]; 
const int STA_HASH = 0;
const int STA_GEN_CAP = 1;
const int STA_WELLEAT = 2;
const int STA_KA = 3;
const int STA_KB = 4;
const int STA_NULL = 5;
const int STA_RE = 6;
const bool NEXT_WHOLE = true;    
const bool ROOT_UNI = true; 

struct myMoveSort {
    int staNum, mvsIndex, mvsNum;
    int mvHash, mvKillerA, mvKillerB;
    myMove mvs[MAX_MOVS];

    void setHisTab(void);
    void mvsSort(void);  
    bool wellEat(const myPosition& pos, int mv) {
        bool tb = mv == 0 || staNum == STA_WELLEAT || (staNum < STA_WELLEAT&& pos.wellEat(mv));
        return tb;
    }

    // 静态搜索着法排序
    void genAll(const myPosition& pos) {
        mvsIndex = 0;
        mvsNum = pos.genAllMvs(mvs);
        setHisTab();
        mvsSort();
    }
    void initQ(const myPosition& pos) {
        mvsIndex = 0;
        mvsNum = pos.genEatMvs(mvs);
        mvsSort();
    }
    int getNextQ(bool ifNA = false) {
        if (mvsIndex < mvsNum && (ifNA || mvs[mvsIndex].wsc > 0)) {
            mvsIndex++;
            return mvs[mvsIndex - 1].bmv;
        }
        else  return 0;
    }

    // 完全搜索着法排序
    void initFS(const myPosition& pos, int mv, const uint16_t* lpwmvKiller) {
        staNum = STA_HASH;
        mvHash = mv;
        mvKillerA = lpwmvKiller[0];
        mvKillerB = lpwmvKiller[1];
    }
    int initEvade(myPosition& pos, int mv, const uint16_t* lpwmvKiller);
    int nextFS(const myPosition& pos);

    // 根结点着法排序
    void initRM(const myPosition& pos);
    void resetRM(bool ifUni = false) {
        mvsIndex = 0;
        mvsSort();
        mvsIndex = (ifUni ? 1 : 0);
    }
    int nextRM(void) {
        if (mvsIndex < mvsNum) {
            mvsIndex++;
            return mvs[mvsIndex - 1].bmv;
        }
        else return 0;
    }
    void flashRM(int mv);
};
inline void emptyHisTab(void) {
    memset(HisTable, 0, 65536*4);
}
inline void emptyKillTab(uint16_t(*lpwmvKiller)[2]) {
    memset(lpwmvKiller, 0, MAX_DEP * sizeof(uint16_t[2]));
}
inline void dupKillTab(uint16_t(*lpwmvDst)[2], const uint16_t(*lpwmvSrc)[2]) {
    memcpy(lpwmvDst, lpwmvSrc, MAX_DEP * sizeof(uint16_t[2]));
}
     
inline void setBMv(int mv, int nDepth, uint16_t* lpwmvKiller) {
    HisTable[mv] += sqr(nDepth);
    if (lpwmvKiller[0] != mv) {
        lpwmvKiller[1] = lpwmvKiller[0];
        lpwmvKiller[0] = mv;
    }
}

#endif