#pragma once
#ifndef PREEVAL_H
#define PREEVAL_H

// 扩展的局面预评价结构
extern struct PreEvalStructEx {
	int vlBlackAdvisorLeakage, vlWhiteAdvisorLeakage;
	int vlHollowThreat[16], vlCentralThreat[16];
	int vlWhiteBottomThreat[16], vlBlackBottomThreat[16];
	char cPopCnt16[65536]; // 加速PopCnt16的数组，只需要初始化一次
}PreEvalEx;

#endif