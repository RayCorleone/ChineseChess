#include "define.h"
#include "rc4prng.h"
#include "ucci.h"
#include "pregen.h"
#include "position.h"

#ifndef SEARCH_H
#define SEARCH_H
//��������
struct mySearch {
	bool ifQui;							// �˳���
	bool ifHT, ifBook;					// ��ϣ�û������ֿ⣿
	bool ifNM, ifKnow;					
	int maxTime;						//���ʱ��
	int randMNum;					
	char szBookFile[1024];				
	myPosition pos;					// ����������
	myTool randomNum;				
};

extern mySearch Search;

// UCCI���湹�����
void BuildPos(myPosition &pos, const UCStruct &UcciComm);

// ��������������
void SearchMain(int nDepth);

#endif
