#include "position.h"
#include "book.h"

int getBookMvs(const myPosition& pos, myBook* lpbks) {
    myBookData BookData;
    myPosition posScan;
    myBook bk;
    int nScan, nLow, nHigh, index;
    int i, j, nMoves;
    // 使用开局库
    BookData.init();
    //搜索局面
    posScan = pos;
    for (nScan = 0; nScan < 2; nScan++) {
        index = nLow = 0;
        nHigh = BookData.len - 1;
        while (nLow <= nHigh) {
            index = (nLow + nHigh) / 2;
            BookData.Read(bk, index);
            if (IF_EXIST(bk, posScan) < 0) {
                nLow = index + 1;
            }
            else if (IF_EXIST(bk, posScan) > 0) {
                nHigh = index - 1;
            }
            else {
                break;
            }
        }
        if (nLow <= nHigh) {
            break;
        }
        // 原、镜像局面均要搜索
        posScan.symSitu();
    }

    // 无匹配，返回空着；
    if (nScan == 2) {
        return 0;
    }

    // 有匹配，向前查找第一个着法
    for (index--; index >= 0; index--) {
        BookData.Read(bk, index);
        if (IF_EXIST(bk, posScan) < 0) {
            break;
        }
    }

    // 向后依次读入每个着法
    nMoves = 0;
    for (index++; index < BookData.len; index++) {
        BookData.Read(bk, index);
        if (IF_EXIST(bk, posScan) > 0) {
            break;
        }
        if (posScan.ifLegalM(bk.bmv)) {
            // 如果局面是第二趟搜索到的，则着法镜像
            lpbks[nMoves].index = index;
            lpbks[nMoves].bmv = (nScan == 0 ? bk.bmv : symM(bk.bmv));
            lpbks[nMoves].wsc = bk.wsc;
            nMoves++;
            if (nMoves == MAX_MOVS) {
                break;
            }
        }
    }

    //按分选择
    for (i = 0; i < nMoves - 1; i++) {
        for (j = nMoves - 1; j > i; j--) {
            if (lpbks[j - 1].wsc < lpbks[j].wsc) {
                swap(lpbks[j - 1], lpbks[j]);
            }
        }
    }
    return nMoves;
}