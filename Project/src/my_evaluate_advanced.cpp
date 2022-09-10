#include"my_evaluate.h"

extern const myPosition* pPublicPos=NULL;

extern const int GENERAL_TY;
extern const int OFFIER_TY;
extern const int ELE_TY;
extern const int H_TY;
extern const int JU_TYPE;
extern const int CANNON_TYPE;
extern const int PAWN_TYPE;
extern const int PieceTypes[48];

//extern PreEvalStruct PreEval;
//extern PreEvalStructEx PreEvalEx;

#ifdef USE_BIT_PIECE
extern const int RANK_TOP;
extern const int RANK_BOTTOM;
extern const int FILE_LEFT;
extern const int FILE_CENTER;
extern const int FILE_RIGHT;

inline static int _piece_type(int pc) {
	return PieceTypes[pc];
}
inline static int _reverse_sq(int sq)
{
	return 254 - sq;
}

inline static int _rank_y(int sq) {
	return sq >> 4;
}
inline static int _file_x(int sq) {
	return sq & 15;
}
inline static int _coord_xy(int _rank, int _file) {
	return (_rank << 4) + _file;
}
inline static bool _same_half(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x80) == 0;
}
#endif USE_BIT_PIECE

extern const int BEST_SCORE;

//存放棋子威胁分数
//KING,ADVISOR,BISHOP,KNIGHT,ROOK,CANNON,PAWN
static const int ThreatValue[7] = {
	10000,250,250,350,500,350,100
};

//存放棋子灵活性分数
//KING,ADVISOR,BISHOP,KNIGHT,ROOK,CANNON,PAWN
static const int FlexValue[7] = {
	0,1,1,12,6,6,15
};

//存放兵过河值加成
static const uint8_t vlPawnAttacking[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 90, 90,110,120,120,120,110, 90, 90,  0,  0,  0,  0,
  0,  0,  0, 90, 90,110,120,120,120,110, 90, 90,  0,  0,  0,  0,
  0,  0,  0, 70, 90,110,110,110,110,110, 90, 70,  0,  0,  0,  0,
  0,  0,  0, 70, 70, 70, 70, 70, 70, 70, 70, 70,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};
inline static int GetPawnAtkValue(int sq, int pc) {
	return pc == PAWN_TYPE + 16 ? vlPawnAttacking[sq] : (pc == PAWN_TYPE + 32 ? vlPawnAttacking[_reverse_sq(sq)] : 0);
}


int16_t AttackPos[256];	//存放每一位置被威胁的信息
uint8_t GuardPos[256];	//存放每一位置被保护的信息
uint8_t FlexibilityPos[256];	//存放每一位置上棋子灵活性的信息
int ChessValue[256];	//存放每一位置上棋子总价值的信息
int RelateCnt;	//记录一棋子相关位置个数
int sqRelate[20];	//记录一棋子相关位置


//调用PreEvaluate/AdvEvaluate进行评价
//输入PositionStruct::side    0表示红方，1表示黑方
//                    ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//                    ucsqPieces[48]  每个棋子放的位置，0表示被吃
//                    dwBitPiece    32位的棋子位，0到31位依次表示序号为16到47的棋子是否还在棋盘上
//                    redScore, blackScore  红方和黑方的子力价值             
int outer_Evaluate(const bool side, const uint8_t* squares, const uint8_t* pieces, const uint32_t bitPiece, const int redScore,const int blackScore, const myPosition* pPos)
{
	pPublicPos = pPos;
	int value = side ? blackScore - redScore : redScore - blackScore;
	value += PreEval.vlAdvanced;
	value += pPos->AdvisorShape();;
	value += int(round(ALTERNATIVE_EVALUATION_WEIGHT * AdvEvaluate(side, squares)));
	return value;
}

//输入PositionStruct::side    0表示红方，1表示黑方
//                    ucpcSquares[256]  每个格子放的棋子，0表示没有棋子          
int AdvEvaluate(const bool side, const uint8_t* squares)
{
	int i, j, k;
	int pcChess, pcTarget;

	memset(ChessValue, 0, sizeof(ChessValue));
	memset(AttackPos, 0, sizeof(AttackPos));
	memset(GuardPos, 0, sizeof(GuardPos));
	memset(FlexibilityPos, 0, sizeof(FlexibilityPos));

	//扫描棋盘，找出每个棋子即威胁保护的棋子，还有灵活性
	for (i = RANK_TOP; i <= RANK_BOTTOM; i++)	//rank
		for (j = FILE_LEFT; j <= FILE_RIGHT; j++)	//file
		{
			if (squares[_coord_xy(i, j)] != 0)
			{
				pcChess = squares[_coord_xy(i, j)];
				RelatePiece(squares, j, i);
				for (k = 0; k < RelateCnt; k++)
				{
					pcTarget = squares[sqRelate[k]];
					if (pcTarget == 0)
						FlexibilityPos[_coord_xy(i, j)]++;
					else
					{
						if (_same_half(pcChess, pcTarget))
							GuardPos[sqRelate[k]]++;
						else
						{
							AttackPos[sqRelate[k]]++;
							FlexibilityPos[_coord_xy(i, j)]++;
							if (pcTarget == GENERAL_TY + 16)
							{
								if (side)
									return BEST_SCORE;
							}
							else if (pcTarget == GENERAL_TY + 32)
							{
								if (!side)
									return BEST_SCORE;
							}
							else	//根据威胁棋子加上威胁分值
								AttackPos[sqRelate[k]] += (30 + (ThreatValue[_piece_type(pcTarget)] - ThreatValue[_piece_type(pcChess)]) / 10) / 10;
						}
					}
				}
			}
		}

	//循环统计扫描到的数据
	for (i = RANK_TOP; i <= RANK_BOTTOM; i++)	//rank
		for (j = FILE_LEFT; j <= FILE_RIGHT; j++)	//file
		{
			if (squares[_coord_xy(i, j)] != 0)
			{
				pcChess = squares[_coord_xy(i, j)];
				ChessValue[_coord_xy(i, j)]++;
				ChessValue[_coord_xy(i, j)] += FlexValue[_piece_type(pcChess)] * FlexibilityPos[_coord_xy(i, j)];
				ChessValue[_coord_xy(i, j)] += GetPawnAtkValue(_coord_xy(i, j), squares[_coord_xy(i, j)]);
			}
		}
	//继续统计
	int nHalfvalue;
	for (i = RANK_TOP; i <= RANK_BOTTOM; i++)	//rank
		for (j = FILE_LEFT; j <= FILE_RIGHT; j++)	//file
		{
			if (squares[_coord_xy(i, j)] != 0)
			{
				pcChess = squares[_coord_xy(i, j)];
				//棋子威胁1/16计入威胁保护增量
				nHalfvalue = ThreatValue[_piece_type(pcChess)] / 16;
				//棋子基本价值计入分值
				//ChessValue[_coord_xy(i, j)] += ThreatValue[_piece_type(pcChess)];

				if (pcChess < 32)
				{
					if (AttackPos[_coord_xy(i, j)])
					{
						if (!side)
						{
							if (pcChess == GENERAL_TY + 16)
								ChessValue[_coord_xy(i, j)] -= 20;
							else
							{
								ChessValue[_coord_xy(i, j)] -= nHalfvalue * 2;
								if (GuardPos[_coord_xy(i, j)])
									ChessValue[_coord_xy(i, j)] += nHalfvalue;
							}
						}
						else
						{
							if (pcChess == GENERAL_TY + 16)
								return BEST_SCORE;
							ChessValue[_coord_xy(i, j)] -= nHalfvalue * 10;
							if (GuardPos[_coord_xy(i, j)])
								ChessValue[_coord_xy(i, j)] += nHalfvalue * 9;
						}
						ChessValue[_coord_xy(i, j)] -= AttackPos[_coord_xy(i, j)];
					}
					else
						if (GuardPos[_coord_xy(i, j)])
							ChessValue[_coord_xy(i, j)] += 5;
				}
				else
				{
					if (AttackPos[_coord_xy(i, j)])
					{
						if (side)
						{
							if (pcChess == GENERAL_TY + 32)
								ChessValue[_coord_xy(i, j)] -= 20;
							else
							{
								ChessValue[_coord_xy(i, j)] -= nHalfvalue * 2;
								if (GuardPos[_coord_xy(i, j)])
									ChessValue[_coord_xy(i, j)] += nHalfvalue;
							}
						}
						else
						{
							if (pcChess == GENERAL_TY + 32)
								return BEST_SCORE;
							ChessValue[_coord_xy(i, j)] -= nHalfvalue * 10;
							if (GuardPos[_coord_xy(i, j)])
								ChessValue[_coord_xy(i, j)] += nHalfvalue * 9;
						}
						ChessValue[_coord_xy(i, j)] -= AttackPos[_coord_xy(i, j)];
					}
					else
						if (GuardPos[_coord_xy(i, j)])
							ChessValue[_coord_xy(i, j)] += 5;
				}
			}
		}
	//以上统计两方棋子总价值

	//统计两方总分
	int nWhiteValue = 0;
	int	nBlackValue = 0;

	for (i = RANK_TOP; i <= RANK_BOTTOM; i++)	//rank
		for (j = FILE_LEFT; j <= FILE_RIGHT; j++)	//file
		{
			pcChess = squares[_coord_xy(i, j)];
			if (pcChess != 0)
			{
				if (pcChess < 32)
					nWhiteValue += ChessValue[_coord_xy(i, j)];
				else
					nBlackValue += ChessValue[_coord_xy(i, j)];
			}
		}

	if (!side)
		return nWhiteValue - nBlackValue;
	else
		return  nBlackValue - nWhiteValue;
}


//将一个位置加入sqRelate中
//输入：_rank  纵坐标
//      _file   横坐标
void sqRelateAppend(int _rank, int _file)
{
	sqRelate[RelateCnt] = _coord_xy(_rank, _file);
	RelateCnt++;
}


//枚举位置上棋子所有有关位置，包括可走到的位置和可保护的位置
//输入：PositionStruct::ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//      _rank   纵坐标
//      _file   横坐标
int RelatePiece(const uint8_t* squares, int _file, int _rank)
{
	RelateCnt = 0;
	int pcChess;
	bool judgeFlag;
	int y, x;	//rank,file

	pcChess = squares[_coord_xy(_rank, _file)];
	if (_piece_type(pcChess) == GENERAL_TY)
	{
		for (y = RANK_TOP; y < 3 + RANK_TOP; y++)
			for (x = 3 + FILE_LEFT; x < 6 + FILE_LEFT; x++)
				if (CanTouch(squares, _rank, _file, y, x))
					sqRelateAppend(x, y);
		for (y = 7 + RANK_TOP; y <= RANK_BOTTOM; y++)
			for (x = 3 + FILE_LEFT; x < 6 + FILE_LEFT; x++)
				if (CanTouch(squares, _rank, _file, y, x))
					sqRelateAppend(x, y);
	}
	else if (_piece_type(pcChess) == OFFIER_TY)
	{
		if (pcChess < 32)
		{
			for (y = 7 + RANK_TOP; y <= RANK_BOTTOM; y++)
				for (x = 3 + FILE_LEFT; x < 6 + FILE_LEFT; x++)
					if (CanTouch(squares, _rank, _file, y, x))
						sqRelateAppend(x, y);
		}
		else
		{
			for (y = RANK_TOP; y < 3 + RANK_TOP; y++)
				for (x = 3 + FILE_LEFT; x < 6 + FILE_LEFT; x++)
					if (CanTouch(squares, _rank, _file, y, x))
						sqRelateAppend(x, y);
		}
	}
	if (_piece_type(pcChess) == ELE_TY)
	{
		x = _file + 2;
		y = _rank + 2;
		if (x <= FILE_RIGHT && y <= RANK_BOTTOM && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file + 2;
		y = _rank - 2;
		if (x <= FILE_RIGHT && y >= RANK_TOP && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file - 2;
		y = _rank + 2;
		if (x >= FILE_LEFT && y <= RANK_BOTTOM && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file - 2;
		y = _rank - 2;
		if (x >= FILE_LEFT && y >= RANK_TOP && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);
	}
	if (_piece_type(pcChess) == H_TY)
	{
		x = _file + 2;
		y = _rank + 1;
		if ((x <= FILE_RIGHT && y <= RANK_BOTTOM) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file + 2;
		y = _rank - 1;
		if ((x <= FILE_RIGHT && y >= RANK_TOP) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file - 2;
		y = _rank + 1;
		if ((x >= FILE_LEFT && y <= RANK_BOTTOM) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file - 2;
		y = _rank - 1;
		if ((x >= FILE_LEFT && y >= RANK_TOP) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);

		x = _file + 1;
		y = _rank + 2;
		if ((x <= FILE_RIGHT && y <= RANK_BOTTOM) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);
		x = _file - 1;
		y = _rank + 2;
		if ((x >= FILE_LEFT && y <= RANK_BOTTOM) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);
		x = _file + 1;
		y = _rank - 2;
		if ((x <= FILE_RIGHT && y >= RANK_TOP) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);
		x = _file - 1;
		y = _rank - 2;
		if ((x >=  FILE_LEFT && y >= RANK_TOP) && CanTouch(squares, _rank, _file, y, x))
			sqRelateAppend(x, y);
	}
	if (_piece_type(pcChess) == JU_TYPE)
	{
		x = _file + 1;
		y = _rank;
		while (x <= FILE_RIGHT)
		{
			if (0 == squares[_coord_xy(y,x)])
				sqRelateAppend(x, y);
			else
			{
				sqRelateAppend(x, y);
				break;
			}
			x++;
		}

		x = _file - 1;
		y = _rank;
		while (x >=  FILE_LEFT)
		{
			if (0 == squares[_coord_xy(y,x)])
				sqRelateAppend(x, y);
			else
			{
				sqRelateAppend(x, y);
				break;
			}
			x--;
		}

		x = _file;
		y = _rank + 1;
		while (y <= RANK_BOTTOM)
		{
			if (0 == squares[_coord_xy(y,x)])
				sqRelateAppend(x, y);
			else
			{
				sqRelateAppend(x, y);
				break;
			}
			y++;
		}

		x = _file;
		y = _rank - 1;
		while (y >= RANK_TOP)
		{
			if (0 == squares[_coord_xy(y,x)])
				sqRelateAppend(x, y);
			else
			{
				sqRelateAppend(x, y);
				break;
			}
			y--;
		}
	}
	if (_piece_type(pcChess) == PAWN_TYPE)
	{
		if (pcChess < 32)
		{
			y = _rank - 1;
			x = _file;

			if (y >= RANK_TOP)
				sqRelateAppend(x, y);

			if (_rank < 5 + RANK_TOP)
			{
				y = _rank;
				x = _file + 1;
				if (x <= FILE_RIGHT)
					sqRelateAppend(x, y);
				x = _file - 1;
				if (x >=  FILE_LEFT)
					sqRelateAppend(x, y);
			}
			else
			{
				y = _rank + 1;
				x = _file;

				if (y <= RANK_BOTTOM)
					sqRelateAppend(x, y);

				if (_rank > 4 + RANK_TOP)
				{
					y = _rank;
					x = _file + 1;
					if (x <= FILE_RIGHT)
						sqRelateAppend(x, y);
					x = _file - 1;
					if (x >=  FILE_LEFT)
						sqRelateAppend(x, y);
				}
			}
		}
		if (_piece_type(pcChess) == CANNON_TYPE)
		{
			x = _file + 1;
			y = _rank;
			judgeFlag = false;
			while (x <= FILE_RIGHT)
			{
				if (0 == squares[_coord_xy(y,x)])
				{
					if (!judgeFlag)
						sqRelateAppend(x, y);
				}
				else
				{
					if (!judgeFlag)
						judgeFlag = true;
					else
					{
						sqRelateAppend(x, y);
						break;
					}
				}
				x++;
			}

			x = _file - 1;
			judgeFlag = false;
			while (x >=  FILE_LEFT)
			{
				if (0 == squares[_coord_xy(y,x)])
				{
					if (!judgeFlag)
						sqRelateAppend(x, y);
				}
				else
				{
					if (!judgeFlag)
						judgeFlag = true;
					else
					{
						sqRelateAppend(x, y);
						break;
					}
				}
				x--;
			}
			x = _file;
			y = _rank + 1;
			judgeFlag = false;
			while (y <= RANK_BOTTOM)
			{
				if (0 == squares[_coord_xy(y,x)])
				{
					if (!judgeFlag)
						sqRelateAppend(x, y);
				}
				else
				{
					if (!judgeFlag)
						judgeFlag = true;
					else
					{
						sqRelateAppend(x, y);
						break;
					}
				}
				y++;
			}

			y = _rank - 1;
			judgeFlag = false;
			while (y >= RANK_TOP)
			{
				if (0 == squares[_coord_xy(y,x)])
				{
					if (!judgeFlag)
						sqRelateAppend(x, y);
				}
				else
				{
					if (!judgeFlag)
						judgeFlag = true;
					else
					{
						sqRelateAppend(x, y);
						break;
					}
				}
				y--;
			}
		}
	}
	return RelateCnt;
}


//判断位于_from的子是否可达到_to
//输入：PositionStruct::ucpcSquares[256]  每个格子放的棋子，0表示没有棋子
//      _rank   纵坐标(_from,_to)
//      _file   横坐标(_from,_to)
bool CanTouch(const uint8_t* squares, int _from_rank, int _from_file, int _to_rank, int _to_file)
{
	int sqSrc = _from_rank * 16 + _from_file;		// 起始地址
	int sqDst = _to_rank * 16 + _to_file;	// 目标地址
	int mv = sqSrc + (sqDst << 8);
	return pPublicPos->ifLegalM(mv);
}

