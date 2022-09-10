#include <string.h>
#include "define.h"
#include "position.h"

#ifndef HASH_H
#define HASH_H

const int HT_BET = 1;
const int HT_ALP = 2;
const int HT_PV = HT_ALP | HT_BET;
const int HT_DEP = 2;  
const int HT_NULL_DEP = 2;    // 空着裁剪   
struct myHashTable {
	uint32_t dwZLockA;           // Zobrist校验锁，防止存取冲突
	uint16_t bmv;                      
	uint8_t alpDep, betDep; 
	int16_t alpScore, betScore;        
	uint32_t dwZLockB;       
}; 
extern int tabSize;              
extern myHashTable *tabP;       
inline void emptyHT(void) {         // 清空置换表
	memset(tabP, 0, (tabSize + 1) * sizeof(myHashTable));
}
inline void genHT(int ts) { // 分配置换表，大小是 2^ts 字节
	tabSize = ((1 << ts) / sizeof(myHashTable)) - 1;
	tabP = new myHashTable[tabSize + 1];
	emptyHT();
}
inline void delTab(void) {           // 释放置换表
	delete[] tabP;
}
inline bool ifZLockEqu(const myHashTable& hsh, const myPosition& pos) {
	return hsh.dwZLockA == pos.zobr.dwLock0 && hsh.dwZLockB == pos.zobr.dwLock1;
}
// 返回置换表项
inline myHashTable& getItem(const myPosition& pos, int layer) {
	return tabP[(pos.zobr.dwKey + layer) & tabSize];
}
//存储
void memHT(const myPosition &pos, int flag, int score, int dep, int move); 
//获取
int getHT(const myPosition &pos, int alpScore, int betScore, int dep, bool unNull, int & move);


#endif