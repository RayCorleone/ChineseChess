#include <iostream>
#include <string.h>
#include "define.h"
#include "ucci.h"

// UCCIָ�����ģ��������UCCIָ���������ɡ�
// ���е�һ��������"BootLine()"��򵥣�ֻ������������������ĵ�һ��ָ��
// ����"ucci"ʱ�ͷ���"UC_UCCI"������һ�ɷ���"UC_DEFULT"
// ǰ�������������ȴ��Ƿ������룬���û��������ִ�д���ָ��"SLEEP()" -> �� base.h ��
// ��������������"BusyLine()"��ֻ��������˼��ʱ������û������ʱֱ�ӷ���"UC_DEFULT"

const int MAX_ROLL = 1024;
const int LINE_INPUT_MAX_CHAR = 8192;

static char szFen[LINE_INPUT_MAX_CHAR];     //fen��
static unsigned int dwCoordList[MAX_ROLL];  //�����ŷ���

// �ж��Ƿ����߷���
static bool IsMoves(char*& sz1, const char* sz2) {
    char* lpsz;
    lpsz = strstr(sz1, sz2);
    if (lpsz == NULL)
        return false;
    else {
        sz1 = lpsz + strlen(sz2);
        return true;
    }
}

// ��position ָ��ӹ�����
static bool ParsePos(UCStruct& Ucci, char* s) {
    int i;
    int end;

    if (_strnicmp(s, "fen ", strlen("fen ")) == 0) { // �ж�FEN��
        s += strlen("fen ");
        end = strcspn(s, "-") - 1;
        strncpy_s(szFen, s, end);
        Ucci.szFenStr = szFen;
    }
    else if (_strnicmp(s, "startpos", strlen("startpos")) == 0) {   // �ж�startpos
        Ucci.szFenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r";
    }
    else
        return false;
    //�����fen����std::cout << "��Fen = " << Ucci.szFenStr << ";" << std::endl;

    // Ѱ���Ƿ���"moves"�ؼ���
    Ucci.mvsNum = 0;
    if (IsMoves(s, " moves ")) {
        *(s - strlen(" moves ")) = '\0';
        Ucci.mvsNum = (int)(strlen(s) + 1) / 5; // ��ʾ��"moves"�����ÿ���ŷ�����1���ո��4���ַ�
        //�����nMove��std::cout << "��MoveNum = " << Ucci.mvsNum << std::endl;
        for (i = 0; i < Ucci.mvsNum; i++) {
            dwCoordList[i] = *(unsigned int*)s; // 4���ַ���ת��Ϊһ��"unsigned int"���洢�ʹ�����������
            s += sizeof(unsigned int) + 1;
        }
        Ucci.lpdwMovesCoord = dwCoordList;

        //�����moves��
        //for (i = 0; i < Ucci.mvsNum; i++)
        //    std::cout << "��Moves " << i + 1 << Ucci.lpdwMovesCoord[i] << std::endl;
    }
    return true;
}

// UCCI���������ĵ�һ��ָ��
UcciCommand BootLine() {
    char szStr[LINE_INPUT_MAX_CHAR];

    while (!std::cin.getline(szStr, LINE_INPUT_MAX_CHAR))
        SLEEP();
    if (strcmp(szStr, "ucci") == 0) // 1."ucci"ָ��
        return UC_UCCI;
    else
        return UC_DEFULT;
}

// �������ʱ����ָ��
UcciCommand IdleLine(UCStruct& Ucci) {
    char szStr[LINE_INPUT_MAX_CHAR];
    char* lp;

    while (!std::cin.getline(szStr, LINE_INPUT_MAX_CHAR))
        SLEEP();
    lp = szStr;

    if (false) {}
    // 2."isready"ָ��
    else if (strcmp(lp, "isready") == 0)
        return UC_READY;
    // 3."position"ָ��
    else if (_strnicmp(lp, "position ", strlen("position ")) == 0) {
        lp += strlen("position ");
        return ParsePos(Ucci, lp) ? UC_POS : UC_DEFULT;
    }
    // 4."go"ָ��
    else if (_strnicmp(lp, "go ", strlen("go ")) == 0) {
        Ucci.nTime = UCCI_TIME_DEFULT; // Ϊ nTime �趨Ĭ��ֵ
        lp += strlen("go ");
        if (_strnicmp(lp, "time ", strlen("time ")) == 0) {   // ��"time"ѡ��, ���� nTime ֵ
            lp += strlen("time ");
            sscanf_s(lp, "%d", &Ucci.nTime);
        }
        //�����ʱ�䣺std::cout << "��Time = " << Ucci.nTime << std::endl;
        return UC_GO;
    }
    else if (strcmp(lp, "quit") == 0)       // 5."quit"ָ��
        return UC_QUIT;
    else
        return UC_DEFULT;
}

// ����˼��ʱ����ָ��
UcciCommand BusyLine(UCStruct& Ucci) {
    char szStr[LINE_INPUT_MAX_CHAR];

    if (std::cin.getline(szStr, LINE_INPUT_MAX_CHAR)) {
        if (false) {}
        else if (strcmp(szStr, "isready") == 0) // 6."isready"ָ��
            return UC_READY;
        else if (strcmp(szStr, "quit") == 0)    // 7."quit"ָ��
            return UC_QUIT;
        else
            return UC_DEFULT;
    }
    else
        return UC_DEFULT;
}