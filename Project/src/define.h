#define _CRT_SECURE_NO_WARNINGS

#pragma once
#ifndef _DEFINE_H
#define _DEFINE_H

#include <cmath>
#include <string.h>
#include <windows.h>
#include <sys/timeb.h>

typedef signed   __int64  int64_t; // ll
typedef unsigned __int64 uint64_t; // qw
typedef signed   __int32  int32_t; // l
typedef unsigned __int32 uint32_t; // dw
typedef signed   __int16  int16_t; // s
typedef unsigned __int16 uint16_t; // w
typedef signed   __int8   int8_t;  // c
typedef unsigned __int8  uint8_t;  // uc

template <typename T> inline T sqr(T a) {
    return a * a;
}

template <typename T> inline void swap(T& a, T& b) {
    T t;
    t = a;
    a = b;
    b = t;
}

//当前时间
inline int64_t getTime() {
    timeb tb;
    ftime(&tb);
    return (int64_t)tb.time * 1000 + tb.millitm;
}

// 等待
inline void SLEEP(void) {
    Sleep(1);
}

//汇编操作
__forceinline int Bsr(uint32_t Operand) {
    __asm {
        bsr eax, Operand;
    }
}

__forceinline int Bsf(uint32_t Operand) {
    __asm {
        bsf eax, Operand;
    }
}

__forceinline uint64_t TimeStampCounter(void) {
    __asm {
        rdtsc;
    }
}

#endif