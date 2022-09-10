#include <string.h>
#include "define.h"
#include "position.h"

#ifndef HASH_H
#define HASH_H

const int HT_BET = 1;
const int HT_ALP = 2;
const int HT_PV = HT_ALP | HT_BET;
const int HT_DEP = 2;  
const int HT_NULL_DEP = 2;    // ���Ųü�   
struct myHashTable {
	uint32_t dwZLockA;           // ZobristУ��������ֹ��ȡ��ͻ
	uint16_t bmv;                      
	uint8_t alpDep, betDep; 
	int16_t alpScore, betScore;        
	uint32_t dwZLockB;       
}; 
extern int tabSize;              
extern myHashTable *tabP;       
inline void emptyHT(void) {         // ����û���
	memset(tabP, 0, (tabSize + 1) * sizeof(myHashTable));
}
inline void genHT(int ts) { // �����û�����С�� 2^ts �ֽ�
	tabSize = ((1 << ts) / sizeof(myHashTable)) - 1;
	tabP = new myHashTable[tabSize + 1];
	emptyHT();
}
inline void delTab(void) {           // �ͷ��û���
	delete[] tabP;
}
inline bool ifZLockEqu(const myHashTable& hsh, const myPosition& pos) {
	return hsh.dwZLockA == pos.zobr.dwLock0 && hsh.dwZLockB == pos.zobr.dwLock1;
}
// �����û�����
inline myHashTable& getItem(const myPosition& pos, int layer) {
	return tabP[(pos.zobr.dwKey + layer) & tabSize];
}
//�洢
void memHT(const myPosition &pos, int flag, int score, int dep, int move); 
//��ȡ
int getHT(const myPosition &pos, int alpScore, int betScore, int dep, bool unNull, int & move);


#endif