#include "define.h"
#include "rc4prng.h"
#include "ucci.h"
#include "pregen.h"
#include "position.h"

#ifndef SEARCH_H
#define SEARCH_H
//搜索参数
struct mySearch {
	bool ifQui;							// 退出？
	bool ifHT, ifBook;					// 哈希置换？开局库？
	bool ifNM, ifKnow;					
	int maxTime;						//最大时长
	int randMNum;					
	char szBookFile[1024];				
	myPosition pos;					// 待搜索局面
	myTool randomNum;				
};

extern mySearch Search;

// UCCI局面构造过程
void BuildPos(myPosition &pos, const UCStruct &UcciComm);

// 搜索的启动过程
void SearchMain(int nDepth);

#endif
