#pragma once
#ifndef _UCCI_H_
#define _UCCI_H_

const int UCCI_MAX_DEPTH = 32;      // UCCI����˼���ļ������
const int UCCI_TIME_DEFULT = 60000; // UCCI��ʱ�߷���ÿ����ʼʱ��

// UCCIָ������
enum UcciCommand {
    UC_DEFULT,	// δָ֪��
    UC_UCCI,	// ucci
    UC_READY,	// isready
    UC_POS,		// position {fen <FEN��> | startpos} [moves < �����ŷ��б�>]
    UC_GO,		// go time <milliseconds>
    UC_QUIT		// quit
};

// �����Ϣ��UCCIָ��
union UCStruct {
    // UC_POS: position {fen <FEN��> | startpos} [moves < �����ŷ��б�>]
    struct {
        const char* szFenStr;           // FEN��
        int mvsNum;                   // �����ŷ���
        unsigned int* lpdwMovesCoord;   // �����ŷ�
    };

    // UC_GO: go time <milliseconds>
    struct {
        int nTime;  // ʱ��
    };
};

// ��������������������UCCIָ��������ڲ�ͬ����
UcciCommand BootLine();                 // UCCI���������ĵ�һ��ָ�ֻ����"ucci"
UcciCommand IdleLine(UCStruct& Ucci);   // �������ʱ����ָ��
UcciCommand BusyLine(UCStruct& Ucci);   // ����˼��ʱ����ָ��

#endif // !_UCCI_H_