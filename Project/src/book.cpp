#include "position.h"
#include "book.h"

int getBookMvs(const myPosition& pos, myBook* lpbks) {
    myBookData BookData;
    myPosition posScan;
    myBook bk;
    int nScan, nLow, nHigh, index;
    int i, j, nMoves;
    // ʹ�ÿ��ֿ�
    BookData.init();
    //��������
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
        // ԭ����������Ҫ����
        posScan.symSitu();
    }

    // ��ƥ�䣬���ؿ��ţ�
    if (nScan == 2) {
        return 0;
    }

    // ��ƥ�䣬��ǰ���ҵ�һ���ŷ�
    for (index--; index >= 0; index--) {
        BookData.Read(bk, index);
        if (IF_EXIST(bk, posScan) < 0) {
            break;
        }
    }

    // ������ζ���ÿ���ŷ�
    nMoves = 0;
    for (index++; index < BookData.len; index++) {
        BookData.Read(bk, index);
        if (IF_EXIST(bk, posScan) > 0) {
            break;
        }
        if (posScan.ifLegalM(bk.bmv)) {
            // ��������ǵڶ����������ģ����ŷ�����
            lpbks[nMoves].index = index;
            lpbks[nMoves].bmv = (nScan == 0 ? bk.bmv : symM(bk.bmv));
            lpbks[nMoves].wsc = bk.wsc;
            nMoves++;
            if (nMoves == MAX_MOVS) {
                break;
            }
        }
    }

    //����ѡ��
    for (i = 0; i < nMoves - 1; i++) {
        for (j = nMoves - 1; j > i; j--) {
            if (lpbks[j - 1].wsc < lpbks[j].wsc) {
                swap(lpbks[j - 1], lpbks[j]);
            }
        }
    }
    return nMoves;
}