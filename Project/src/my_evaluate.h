#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include"pregen.h"
#include"preeval.h"
#include"position.h"
using namespace std;

//��������BIT_PIECE���ر���һ��
#define USE_BIT_PIECE

//��������USE_INNER_VALUE���ر���һ��
#define USE_INNER_VALUE

//������ȡ����������ǣ�Ʊ����Ĺ�ϵ�������������̺���
#undef DISABLE_ALTERNATIVE_EVALUATION
//����ǣ�Ʊ����Ĺ�ϵȨ�أ�Ĭ��0.3
#define ALTERNATIVE_EVALUATION_WEIGHT (double)0.15

#undef USE_PRE_VALUE

#define USE_MINE

//extern const myPosition* pPublicPos;

//��ʼ������Ԥ��������(PreEval��PreEvalEx)
//����PositionStruct::ucsqPieces[48]  ÿ�����ӷŵ�λ�ã�0��ʾ����
//                    dwBitPiece    32λ������λ��0��31λ���α�ʾ���Ϊ16��47�������Ƿ���������
//                    redScore, blackScore  �췽�ͺڷ���������ֵ
void outer_PreEvaluate(const uint8_t* pieces, const uint32_t bitPiece, int& redScore, int& blackScore);

//����PreEvaluate/AdvEvaluate��������
//����PositionStruct::side    0��ʾ�췽��1��ʾ�ڷ�
//                    ucpcSquares[256]  ÿ�����ӷŵ����ӣ�0��ʾû������
//                    ucsqPieces[48]  ÿ�����ӷŵ�λ�ã�0��ʾ����
//                    dwBitPiece    32λ������λ��0��31λ���α�ʾ���Ϊ16��47�������Ƿ���������
//                    redScore, blackScore  �췽�ͺڷ���������ֵ             
int outer_Evaluate(const bool side,const uint8_t *squares, const uint8_t* pieces, const uint32_t bitPiece, const int redScore, const int blackScore,const myPosition* pPos);

//����PositionStruct::side    0��ʾ�췽��1��ʾ�ڷ�
//                    ucpcSquares[256]  ÿ�����ӷŵ����ӣ�0��ʾû������          
int AdvEvaluate(const bool side, const uint8_t* squares);

//��һ��λ�ü���RelatePos��
//���룺_rank  ������
//      _file   ������
void sqRelateAppend(int _rank, int _file);

//ö��λ�������������й�λ�ã��������ߵ���λ�úͿɱ�����λ��
//���룺PositionStruct::ucpcSquares[256]  ÿ�����ӷŵ����ӣ�0��ʾû������
//      _rank   ������
//      _file   ������
int RelatePiece(const uint8_t* squares, int j, int i);

//�ж�λ��_from�����Ƿ�ɴﵽ_to
//���룺PositionStruct::ucpcSquares[256]  ÿ�����ӷŵ����ӣ�0��ʾû������
//      _rank   ������(_from,_to)
//      _file   ������(_from,_to)
bool CanTouch(const uint8_t* squares, int _from_rank, int _from_file, int _to_rank, int _to_file);