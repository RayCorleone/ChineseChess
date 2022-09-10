#include"my_evaluate.h"

//��������BIT_PIECE���ر���һ��
#ifdef USE_BIT_PIECE
// ÿ�����������ͱ��
extern const int GENERAL_TY;
extern const int OFFIER_TY;
extern const int ELE_TY;
extern const int H_TY;
extern const int JU_TYPE;
extern const int CANNON_TYPE;
extern const int PAWN_TYPE;

// ÿ�������Ŀ�ʼ��źͽ������
extern const int GENERAL_FROM;
extern const int OFFIER_FROM;
extern const int OFFIER_TO;
extern const int ELE_FROM;
extern const int ELE_TO;
extern const int H_FROM;
extern const int H_TO;
extern const int JU_FROM;
extern const int JU_TO;
extern const int CANNON_FROM;
extern const int CANNON_TO;
extern const int PAWN_FROM;
extern const int PAWN_TO;

// ��������������λ
extern const int GENERAL_BITPIECE;
extern const int OFFIER_BITPIECE;
extern const int ELE_BITPIECE;
extern const int H_BITPIECE;
extern const int JU_BITPIECE;
extern const int CANNON_BITPIECE;
extern const int PAWN_BITPIECE;
extern const int ATTACK_BITPIECE;

//��λ��
extern const int PieceTypes[48] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6,
  0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6
};

inline bool _white_half(int sq)
{
	return (sq & 0x80) != 0;
}
inline bool _black_half(int sq)
{
	return (sq & 0x80) == 0;
}

//λ���̺ϳ�
inline static uint32_t _bit_piece(const int _bit_choose)
{
	return 1 << (_bit_choose - 16);
}

inline static uint32_t _white_bit_piece(const int _bit_piece)
{
	return _bit_piece;
}

inline static uint32_t _black_bit_piece(const int _bit_piece)
{
	return  _bit_piece << 16;
}

inline static uint32_t _both_bit_piece(const int _bit_piece)
{
	return _white_bit_piece(_bit_piece) | _black_bit_piece(_bit_piece);
}

//λ1����
inline static size_t _popcnt(const uint8_t v)
{
	static const size_t CountTable[256] = {
		0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8 };
	return CountTable[v];
}
inline static size_t _popcnt(const uint16_t v)
{
	return _popcnt((uint8_t)v) + _popcnt((uint8_t)(v >> 8));
}
inline static size_t _popcnt(const uint32_t v)
{
	return _popcnt((uint8_t)v) + _popcnt((uint8_t)(v >> 8)) + _popcnt((uint8_t)(v >> 16)) + _popcnt((uint8_t)(v >> 24));
}
inline static size_t _popcnt(const uint64_t v)
{
	return _popcnt((uint8_t)v) + _popcnt((uint8_t)(v >> 8)) + _popcnt((uint8_t)(v >> 16)) + _popcnt((uint8_t)(v >> 24)) + _popcnt((uint8_t)(v >> 32)) + _popcnt((uint8_t)(v >> 40)) + _popcnt((uint8_t)(v >> 48)) + _popcnt((uint8_t)(v >> 56));
}

inline static int _reverse_sq(int sq)
{
	return 254 - sq;
}
#endif USE_BIT_PIECE


extern PreEvalStruct PreEval;
PreEvalStructEx PreEvalEx;


#ifdef USE_INNER_VALUE
// 1. ���о֡��н��������˧(��)�ͱ�(��)�����ա������񵰡�
static const uint8_t vlKingPawnMidgameAttacking[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  9,  9,  9, 11, 13, 11,  9,  9,  9,  0,  0,  0,  0,
  0,  0,  0, 39, 49, 69, 84, 89, 84, 69, 49, 39,  0,  0,  0,  0,
  0,  0,  0, 39, 49, 64, 74, 74, 74, 64, 49, 39,  0,  0,  0,  0,
  0,  0,  0, 39, 46, 54, 59, 61, 59, 54, 46, 39,  0,  0,  0,  0,
  0,  0,  0, 29, 37, 41, 54, 59, 54, 41, 37, 29,  0,  0,  0,  0,
  0,  0,  0,  7,  0, 13,  0, 16,  0, 13,  0,  7,  0,  0,  0,  0,
  0,  0,  0,  7,  0,  7,  0, 15,  0,  7,  0,  7,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0, 11, 15, 11,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 2. ���о֡�û�н��������˧(��)�ͱ�(��)
static const uint8_t vlKingPawnMidgameAttackless[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  9,  9,  9, 11, 13, 11,  9,  9,  9,  0,  0,  0,  0,
  0,  0,  0, 19, 24, 34, 42, 44, 42, 34, 24, 19,  0,  0,  0,  0,
  0,  0,  0, 19, 24, 32, 37, 37, 37, 32, 24, 19,  0,  0,  0,  0,
  0,  0,  0, 19, 23, 27, 29, 30, 29, 27, 23, 19,  0,  0,  0,  0,
  0,  0,  0, 14, 18, 20, 27, 29, 27, 20, 18, 14,  0,  0,  0,  0,
  0,  0,  0,  7,  0, 13,  0, 16,  0, 13,  0,  7,  0,  0,  0,  0,
  0,  0,  0,  7,  0,  7,  0, 15,  0,  7,  0,  7,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0, 11, 15, 11,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 3. �о֡��н��������˧(��)�ͱ�(��)
static const uint8_t vlKingPawnEndgameAttacking[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 10, 10, 10, 15, 15, 15, 10, 10, 10,  0,  0,  0,  0,
  0,  0,  0, 50, 55, 60, 85,100, 85, 60, 55, 50,  0,  0,  0,  0,
  0,  0,  0, 65, 70, 70, 75, 75, 75, 70, 70, 65,  0,  0,  0,  0,
  0,  0,  0, 75, 80, 80, 80, 80, 80, 80, 80, 75,  0,  0,  0,  0,
  0,  0,  0, 70, 70, 65, 70, 70, 70, 65, 70, 70,  0,  0,  0,  0,
  0,  0,  0, 45,  0, 40, 45, 45, 45, 40,  0, 45,  0,  0,  0,  0,
  0,  0,  0, 40,  0, 35, 40, 40, 40, 35,  0, 40,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  5,  5, 15,  5,  5,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  3,  3, 13,  3,  3,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  1,  1, 11,  1,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 4. �о֡�û�н��������˧(��)�ͱ�(��)
static const uint8_t vlKingPawnEndgameAttackless[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 10, 10, 10, 15, 15, 15, 10, 10, 10,  0,  0,  0,  0,
  0,  0,  0, 10, 15, 20, 45, 60, 45, 20, 15, 10,  0,  0,  0,  0,
  0,  0,  0, 25, 30, 30, 35, 35, 35, 30, 30, 25,  0,  0,  0,  0,
  0,  0,  0, 35, 40, 40, 45, 45, 45, 40, 40, 35,  0,  0,  0,  0,
  0,  0,  0, 25, 30, 30, 35, 35, 35, 30, 30, 25,  0,  0,  0,  0,
  0,  0,  0, 25,  0, 25, 25, 25, 25, 25,  0, 25,  0,  0,  0,  0,
  0,  0,  0, 20,  0, 20, 20, 20, 20, 20,  0, 20,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  5,  5, 13,  5,  5,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  3,  3, 12,  3,  3,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  1,  1, 11,  1,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 5. û����в����(ʿ)����(��)
static const uint8_t vlAdvisorBishopThreatless[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, 20,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 18,  0,  0, 20, 23, 20,  0,  0, 18,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, 20, 20,  0, 20, 20,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 6. �ܵ���в����(ʿ)����(��)�����ա������񵰡�
static const uint8_t vlAdvisorBishopThreatened[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, 40,  0,  0,  0, 40,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 38,  0,  0, 40, 43, 40,  0,  0, 38,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0, 43,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, 40, 40,  0, 40, 40,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 7. ���оֵ������ա������񵰡�
static const uint8_t vlKnightMidgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 90, 90, 90, 96, 90, 96, 90, 90, 90,  0,  0,  0,  0,
  0,  0,  0, 90, 96,103, 97, 94, 97,103, 96, 90,  0,  0,  0,  0,
  0,  0,  0, 92, 98, 99,103, 99,103, 99, 98, 92,  0,  0,  0,  0,
  0,  0,  0, 93,108,100,107,100,107,100,108, 93,  0,  0,  0,  0,
  0,  0,  0, 90,100, 99,103,104,103, 99,100, 90,  0,  0,  0,  0,
  0,  0,  0, 90, 98,101,102,103,102,101, 98, 90,  0,  0,  0,  0,
  0,  0,  0, 92, 94, 98, 95, 98, 95, 98, 94, 92,  0,  0,  0,  0,
  0,  0,  0, 93, 92, 94, 95, 92, 95, 94, 92, 93,  0,  0,  0,  0,
  0,  0,  0, 85, 90, 92, 93, 78, 93, 92, 90, 85,  0,  0,  0,  0,
  0,  0,  0, 88, 85, 90, 88, 90, 88, 90, 85, 88,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 8. �оֵ���
static const uint8_t vlKnightEndgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 92, 94, 96, 96, 96, 96, 96, 94, 92,  0,  0,  0,  0,
  0,  0,  0, 94, 96, 98, 98, 98, 98, 98, 96, 94,  0,  0,  0,  0,
  0,  0,  0, 96, 98,100,100,100,100,100, 98, 96,  0,  0,  0,  0,
  0,  0,  0, 96, 98,100,100,100,100,100, 98, 96,  0,  0,  0,  0,
  0,  0,  0, 96, 98,100,100,100,100,100, 98, 96,  0,  0,  0,  0,
  0,  0,  0, 94, 96, 98, 98, 98, 98, 98, 96, 94,  0,  0,  0,  0,
  0,  0,  0, 94, 96, 98, 98, 98, 98, 98, 96, 94,  0,  0,  0,  0,
  0,  0,  0, 92, 94, 96, 96, 96, 96, 96, 94, 92,  0,  0,  0,  0,
  0,  0,  0, 90, 92, 94, 92, 92, 92, 94, 92, 90,  0,  0,  0,  0,
  0,  0,  0, 88, 90, 92, 90, 90, 90, 92, 90, 88,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 9. ���оֵĳ������ա������񵰡�
static const uint8_t vlRookMidgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,206,208,207,213,214,213,207,208,206,  0,  0,  0,  0,
  0,  0,  0,206,212,209,216,233,216,209,212,206,  0,  0,  0,  0,
  0,  0,  0,206,208,207,214,216,214,207,208,206,  0,  0,  0,  0,
  0,  0,  0,206,213,213,216,216,216,213,213,206,  0,  0,  0,  0,
  0,  0,  0,208,211,211,214,215,214,211,211,208,  0,  0,  0,  0,
  0,  0,  0,208,212,212,214,215,214,212,212,208,  0,  0,  0,  0,
  0,  0,  0,204,209,204,212,214,212,204,209,204,  0,  0,  0,  0,
  0,  0,  0,198,208,204,212,212,212,204,208,198,  0,  0,  0,  0,
  0,  0,  0,200,208,206,212,200,212,206,208,200,  0,  0,  0,  0,
  0,  0,  0,194,206,204,212,200,212,204,206,194,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 10. �оֵĳ�
static const uint8_t vlRookEndgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,182,182,182,184,186,184,182,182,182,  0,  0,  0,  0,
  0,  0,  0,184,184,184,186,190,186,184,184,184,  0,  0,  0,  0,
  0,  0,  0,182,182,182,184,186,184,182,182,182,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,180,180,180,182,184,182,180,180,180,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 11. ���оֵ��ڣ����ա������񵰡�
static const uint8_t vlCannonMidgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,100,100, 96, 91, 90, 91, 96,100,100,  0,  0,  0,  0,
  0,  0,  0, 98, 98, 96, 92, 89, 92, 96, 98, 98,  0,  0,  0,  0,
  0,  0,  0, 97, 97, 96, 91, 92, 91, 96, 97, 97,  0,  0,  0,  0,
  0,  0,  0, 96, 99, 99, 98,100, 98, 99, 99, 96,  0,  0,  0,  0,
  0,  0,  0, 96, 96, 96, 96,100, 96, 96, 96, 96,  0,  0,  0,  0,
  0,  0,  0, 95, 96, 99, 96,100, 96, 99, 96, 95,  0,  0,  0,  0,
  0,  0,  0, 96, 96, 96, 96, 96, 96, 96, 96, 96,  0,  0,  0,  0,
  0,  0,  0, 97, 96,100, 99,101, 99,100, 96, 97,  0,  0,  0,  0,
  0,  0,  0, 96, 97, 98, 98, 98, 98, 98, 97, 96,  0,  0,  0,  0,
  0,  0,  0, 96, 96, 97, 99, 99, 99, 97, 96, 96,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 12. �оֵ���
static const uint8_t vlCannonEndgame[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,100,100,100,100,100,100,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,100,100,100,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,100,100,100,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,102,104,102,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,102,104,102,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,102,104,102,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,102,104,102,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,102,104,102,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,104,106,104,100,100,100,  0,  0,  0,  0,
  0,  0,  0,100,100,100,104,106,104,100,100,100,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};


// ��ͷ�ڵ���в��ֵ��ָ���ǶԺ췽��˵���к�(���ڷ�Ҫ��15ȥ��)�������Ͽ�ͷ��λ��Խ����вԽ�󡣽���о�ʱ����ֵҪ��Ӧ���١�
static const int vlHollowThreat[16] = {
   0,  0,  0,  0,  0,  0, 60, 65, 70, 75, 80, 80, 80,  0,  0,  0
};
static const int vlDecreaseHollowThreat = 2;	//�����ֵ�½�Ϊ	1/vlDecreaseHollowThreat

// �������������в��ֵ��ָ��ͬ�ϣ������ϸ߶�Խ����вԽ��û��������ʱ��ȡ�ķ�֮һ������о�ʱ��ȡֵ�ƺ���Ӧ�仯��
static const int vlCentralThreat[16] = {
   0,  0,  0,  0,  0,  0, 50, 45, 40, 35, 30, 30, 30,  0,  0,  0
};

// �����ڵ���в��ֵ��ָ�����кţ�������Խ����������вԽ����в����ʱ����ֵҪ��Ӧ���١�
static const int vlBottomThreat[16] = {
   0,  0,  0, 40, 30,  0,  0,  0,  0,  0, 30, 40,  0,  0,  0,  0
};
//�����ֵ�½�Ϊ	1/TOTAL_ATTACK_VALUE


static const int ROOK_MIDGAME_VALUE = 6;
static const int KNIGHT_CANNON_MIDGAME_VALUE = 3;
static const int OTHER_MIDGAME_VALUE = 1;
static const int TOTAL_MIDGAME_VALUE = ROOK_MIDGAME_VALUE * 4 + KNIGHT_CANNON_MIDGAME_VALUE * 8 + OTHER_MIDGAME_VALUE * 18;
static const int TOTAL_ADVANCED_VALUE = 4;	//?
static const int TOTAL_ATTACK_VALUE = 8;	//?
static const int TOTAL_ADVISOR_LEAKAGE = 80;
static const int ADVISOR_BISHOP_ATTACKLESS_VALUE = 80;
#endif USE_INNER_VALUE


//��ʼ������Ԥ��������(PreEval��PreEvalEx)
//����PositionStruct::pieces[48]  ÿ�����ӷŵ�λ�ã�0��ʾ����
//                    bitPiece    32λ������λ��0��31λ���α�ʾ���Ϊ16��47�������Ƿ���������
//                    redScore, blackScore  �췽�ͺڷ���������ֵ������ֵ
void outer_PreEvaluate(const uint8_t* pieces, const uint32_t bitPiece, int& redScore, int& blackScore)
{
	const uint16_t* bitPieceWOrB = (uint16_t*)&bitPiece;
	int vlMidGame = 0,    //�оֵ÷֣���ʱ��
		sq, //����λ��0-255
		popwAtk=0, popbAtk=0,   //����������
		vlWhiteSimple, vlBlackSimple;   //Ԥ��ֵ

	uint8_t vlPawnAttacking[256], vlPawnAttackless[256];    //���÷��ؼ���

	//�ж��о֡��о�
	vlMidGame += _popcnt(bitPiece & _both_bit_piece(OFFIER_BITPIECE | ELE_BITPIECE | PAWN_BITPIECE)) * OTHER_MIDGAME_VALUE;
	vlMidGame += _popcnt(bitPiece & _both_bit_piece(H_BITPIECE | CANNON_BITPIECE)) * KNIGHT_CANNON_MIDGAME_VALUE;
	vlMidGame += _popcnt(bitPiece & _both_bit_piece(JU_BITPIECE)) * ROOK_MIDGAME_VALUE;
	vlMidGame = (2 * TOTAL_MIDGAME_VALUE - vlMidGame) * vlMidGame / TOTAL_MIDGAME_VALUE;
	PreEval.vlAdvanced = (TOTAL_ADVANCED_VALUE * vlMidGame + TOTAL_ADVANCED_VALUE / 2) / TOTAL_MIDGAME_VALUE;

	for (sq = 0; sq < 256; sq++)
	{
		if (sq / 16 >= 3 && sq / 16 <= 12 && sq % 16 >= 3 && sq % 16 <= 11)    //in_board
		{
			PreEval.vlWhitePieces[GENERAL_TY][sq] = PreEval.vlBlackPieces[GENERAL_TY][_reverse_sq(sq)] = 
				(vlKingPawnMidgameAttackless[sq] * vlMidGame + vlKingPawnEndgameAttackless[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//˧
			PreEval.vlWhitePieces[H_TY][sq] = PreEval.vlBlackPieces[H_TY][_reverse_sq(sq)] = 
				(vlKnightMidgame[sq] * vlMidGame + vlKnightEndgame[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//��
			PreEval.vlWhitePieces[JU_TYPE][sq] = PreEval.vlBlackPieces[JU_TYPE][_reverse_sq(sq)] =
				(vlRookMidgame[sq] * vlMidGame + vlRookEndgame[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//��
			PreEval.vlWhitePieces[CANNON_TYPE][sq] = PreEval.vlBlackPieces[CANNON_TYPE][_reverse_sq(sq)] = 
				(vlCannonMidgame[sq] * vlMidGame + vlCannonEndgame[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//��
			vlPawnAttacking[sq] =
				(vlKingPawnMidgameAttacking[sq] * vlMidGame + vlKingPawnEndgameAttacking[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//������
			vlPawnAttackless[sq] = 
				(vlKingPawnMidgameAttackless[sq] * vlMidGame + vlKingPawnEndgameAttackless[sq] * (TOTAL_MIDGAME_VALUE - vlMidGame)) / TOTAL_MIDGAME_VALUE;	//���ر�
		}
	}

	//�оֿ�ͷ�ڼ�ֵ�½�������������
	int i;
	for (i = 0; i < 16; i++) 
	{
		PreEvalEx.vlHollowThreat[i] = vlHollowThreat[i] * (vlMidGame*(vlDecreaseHollowThreat-1) + TOTAL_MIDGAME_VALUE) / (TOTAL_MIDGAME_VALUE * vlDecreaseHollowThreat);
		PreEvalEx.vlCentralThreat[i] = vlCentralThreat[i];
	}


	// Ȼ���жϸ����Ƿ��ڽ���״̬�������Ǽ�����ֹ������ӵ����������ճ���2�ڱ�1��ӡ�
	//pieces[i]�ж�û�б���
	for (i = 16 + H_FROM; i <= 16 + JU_TO; i++)
	{
		if (pieces[i] && _black_half(pieces[i]))
			popwAtk += 2;
	}
	for (i = 16 + CANNON_FROM; i <= 16 + PAWN_TO; i++)
	{
		if (pieces[i] && _black_half(pieces[i]))
			popwAtk++;
	}
	for (i = 32 + H_FROM; i <= 32 + JU_TO; i++)
	{
		if (pieces[i] && _white_half(pieces[i]))
			popbAtk += 2;
	}
	for (i = 32 + CANNON_FROM; i <= 32 + PAWN_TO; i++)
	{
		if (pieces[i] && _white_half(pieces[i]))
			popbAtk++;
	}

	// ��������������ȶԷ��࣬��ôÿ��һ������(����2������)��вֵ��2����вֵ��಻����TOTAL_ATTACK_VALUE��
	vlWhiteSimple = _popcnt(uint16_t(bitPieceWOrB[0] & JU_BITPIECE)) * 2 + _popcnt(uint16_t(bitPieceWOrB[0] & (H_BITPIECE | CANNON_BITPIECE)));
	vlBlackSimple = _popcnt(uint16_t(bitPieceWOrB[1] & JU_BITPIECE)) * 2 + _popcnt(uint16_t(bitPieceWOrB[1] & (H_BITPIECE | CANNON_BITPIECE)));
	if (vlWhiteSimple > vlBlackSimple)
		popwAtk += (vlWhiteSimple - vlBlackSimple) * 2;
	else 
		popbAtk += (vlBlackSimple - vlWhiteSimple) * 2;
	popwAtk = min(popwAtk, TOTAL_ATTACK_VALUE);	//������TOTAL_ATTACK_VALUE
	popbAtk = min(popbAtk, TOTAL_ATTACK_VALUE);	//������TOTAL_ATTACK_VALUE
	PreEvalEx.vlBlackAdvisorLeakage = TOTAL_ADVISOR_LEAKAGE * popwAtk / TOTAL_ATTACK_VALUE;
	PreEvalEx.vlWhiteAdvisorLeakage = TOTAL_ADVISOR_LEAKAGE * popbAtk / TOTAL_ATTACK_VALUE;
	for (sq = 0; sq < 256; sq++)
	{
		if (sq / 16 >= 3 && sq / 16 <= 12 && sq % 16 >= 3 && sq % 16 <= 11)    //in_board
		{
			PreEval.vlWhitePieces[OFFIER_TY][sq] = PreEval.vlWhitePieces[ELE_TY][sq] = 
				(vlAdvisorBishopThreatened[sq] * popbAtk + vlAdvisorBishopThreatless[sq] * (TOTAL_ATTACK_VALUE - popbAtk)) / TOTAL_ATTACK_VALUE;
			PreEval.vlBlackPieces[OFFIER_TY][_reverse_sq(sq)] = PreEval.vlBlackPieces[ELE_TY][_reverse_sq(sq)] =
				(vlAdvisorBishopThreatened[sq] * popwAtk + vlAdvisorBishopThreatless[sq] * (TOTAL_ATTACK_VALUE - popwAtk)) / TOTAL_ATTACK_VALUE;
			PreEval.vlWhitePieces[PAWN_TYPE][sq] = 
				(vlPawnAttacking[sq] * popwAtk + vlPawnAttackless[sq] * (TOTAL_ATTACK_VALUE - popwAtk)) / TOTAL_ATTACK_VALUE;
			PreEval.vlBlackPieces[PAWN_TYPE][_reverse_sq(sq)] =
				(vlPawnAttacking[sq] * popbAtk + vlPawnAttackless[sq] * (TOTAL_ATTACK_VALUE - popbAtk)) / TOTAL_ATTACK_VALUE;
		}
	}
	for (i = 0; i < 16; i++)
	{
		PreEvalEx.vlWhiteBottomThreat[i] = vlBottomThreat[i] * popbAtk / TOTAL_ATTACK_VALUE;
		PreEvalEx.vlBlackBottomThreat[i] = vlBottomThreat[i] * popwAtk / TOTAL_ATTACK_VALUE;
	}


#ifndef DISABLE_ADVISOR_BISHOP_ADJUST
	// ����������в���ٵ�����(ʿ)��(��)��ֵ
	redScore = ADVISOR_BISHOP_ATTACKLESS_VALUE * (TOTAL_ATTACK_VALUE - popbAtk) / TOTAL_ATTACK_VALUE;
	blackScore = ADVISOR_BISHOP_ATTACKLESS_VALUE * (TOTAL_ATTACK_VALUE - popwAtk) / TOTAL_ATTACK_VALUE;
#endif DISABLE_ADVISOR_BISHOP_ADJUST

	// ������¼�������λ�÷�
	for (i = 16; i < 32; i++)
	{
		sq = pieces[i];
		if (sq != 0)
			redScore += PreEval.vlWhitePieces[PieceTypes[i]][sq];
	}
	for (i = 32; i < 48; i++)
	{
		sq = pieces[i];
		if (sq != 0)
			blackScore += PreEval.vlBlackPieces[PieceTypes[i]][sq];
	}
}