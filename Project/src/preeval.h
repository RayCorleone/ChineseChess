#pragma once
#ifndef PREEVAL_H
#define PREEVAL_H

// ��չ�ľ���Ԥ���۽ṹ
extern struct PreEvalStructEx {
	int vlBlackAdvisorLeakage, vlWhiteAdvisorLeakage;
	int vlHollowThreat[16], vlCentralThreat[16];
	int vlWhiteBottomThreat[16], vlBlackBottomThreat[16];
	char cPopCnt16[65536]; // ����PopCnt16�����飬ֻ��Ҫ��ʼ��һ��
}PreEvalEx;

#endif