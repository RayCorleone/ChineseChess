#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include"pregen.h"
#include"preeval.h"
#include"position.h"
using namespace std;

//如果定义过BIT_PIECE，关闭这一项
#define USE_BIT_PIECE

//如果定义过USE_INNER_VALUE，关闭这一项
#define USE_INNER_VALUE

//定义此项即取消对于棋子牵制保护的关系，加速评价棋盘函数
#undef DISABLE_ALTERNATIVE_EVALUATION
//棋子牵制保护的关系权重，默认0.3
#define ALTERNATIVE_EVALUATION_WEIGHT (double)0.15

#undef USE_PRE_VALUE

#define USE_MINE

//extern const myPosition* pPublicPos;

//初始化局面预评价数据(PreEval和PreEvalEx)
//输入PositionStruct::ucsqPieces[48]  每个棋子放的位置，0表示被吃
//                    dwBitPiece    32位的棋子位，0到31位依次表示序号为16到47的棋子是否还在棋盘上
//                    redScore, blackScore  红方和黑方的子力价值
void outer_PreEvaluate(const uint8_t* pieces, const uint32_t bitPiece, int& redScore, int& blackScore);

//调用PreEvaluate/AdvEvaluate进行评价
//输入PositionStruct::side    0表示红方，1表示黑方
//                    ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//                    ucsqPieces[48]  每个棋子放的位置，0表示被吃
//                    dwBitPiece    32位的棋子位，0到31位依次表示序号为16到47的棋子是否还在棋盘上
//                    redScore, blackScore  红方和黑方的子力价值             
int outer_Evaluate(const bool side,const uint8_t *squares, const uint8_t* pieces, const uint32_t bitPiece, const int redScore, const int blackScore,const myPosition* pPos);

//输入PositionStruct::side    0表示红方，1表示黑方
//                    ucpcSquares[256]  每个格子放的棋子，0表示没有棋子          
int AdvEvaluate(const bool side, const uint8_t* squares);

//将一个位置加入RelatePos中
//输入：_rank  纵坐标
//      _file   横坐标
void sqRelateAppend(int _rank, int _file);

//枚举位置上棋子所有有关位置，包括可走到的位置和可保护的位置
//输入：PositionStruct::ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//      _rank   纵坐标
//      _file   横坐标
int RelatePiece(const uint8_t* squares, int j, int i);

//判断位于_from的子是否可达到_to
//输入：PositionStruct::ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//      _rank   纵坐标(_from,_to)
//      _file   横坐标(_from,_to)
bool CanTouch(const uint8_t* squares, int _from_rank, int _from_file, int _to_rank, int _to_file);