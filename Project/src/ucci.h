#pragma once
#ifndef _UCCI_H_
#define _UCCI_H_

const int UCCI_MAX_DEPTH = 32;      // UCCI引擎思考的极限深度
const int UCCI_TIME_DEFULT = 60000; // UCCI限时走法的每步初始时间

// UCCI指令类型
enum UcciCommand {
    UC_DEFULT,	// 未知指令
    UC_UCCI,	// ucci
    UC_READY,	// isready
    UC_POS,		// position {fen <FEN串> | startpos} [moves < 后续着法列表>]
    UC_GO,		// go time <milliseconds>
    UC_QUIT		// quit
};

// 后接信息的UCCI指令
union UCStruct {
    // UC_POS: position {fen <FEN串> | startpos} [moves < 后续着法列表>]
    struct {
        const char* szFenStr;           // FEN串
        int mvsNum;                   // 后续着法数
        unsigned int* lpdwMovesCoord;   // 后续着法
    };

    // UC_GO: go time <milliseconds>
    struct {
        int nTime;  // 时间
    };
};

// 下面三个函数用来解释UCCI指令，但适用于不同场合
UcciCommand BootLine();                 // UCCI引擎启动的第一条指令，只接收"ucci"
UcciCommand IdleLine(UCStruct& Ucci);   // 引擎空闲时接收指令
UcciCommand BusyLine(UCStruct& Ucci);   // 引擎思考时接收指令

#endif // !_UCCI_H_