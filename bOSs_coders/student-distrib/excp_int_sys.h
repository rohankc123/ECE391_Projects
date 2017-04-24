#ifndef EXCP_INT_SYS_H
#define EXCP_INT_SYS_H

#ifndef ASM

#include "ps2_keyboard.h"
#include "rtc.h"

extern void sys_test(int arg);
extern void exception0_code();
extern void exception1_code();
extern void exception2_code();
extern void exception3_code();
extern void exception4_code();
extern void exception5_code();
extern void exception6_code();
extern void exception7_code();
extern void exception8_code();
extern void exception9_code();
extern void exceptionA_code();
extern void exceptionB_code();
extern void exceptionC_code();
extern void exceptionD_code();
extern void exceptionE_code();
extern void exceptionF_code();
extern void exception10_code();
extern void exception11_code();
extern void exception12_code();
extern void exception13_code();
extern void exception14_code();
extern void exception15_code();
extern void exception16_code();
extern void exception17_code();
extern void exception18_code();
extern void exception19_code();
extern void exception1A_code();
extern void exception1B_code();
extern void exception1C_code();
extern void exception1D_code();
extern void exception1E_code();
extern void exception1F_code();
extern void interrupt20_code();
extern void interrupt21_code();
extern void interrupt22_code();
extern void interrupt23_code();
extern void interrupt24_code();
extern void interrupt25_code();
extern void interrupt26_code();
extern void interrupt27_code();
extern void interrupt28_code();
extern void interrupt29_code();
extern void interrupt2A_code();
extern void interrupt2B_code();
extern void interrupt2C_code();
extern void interrupt2D_code();
extern void interrupt2E_code();
extern void interrupt2F_code();
extern void syscall_code();
extern int prac_sys_call();

#endif

#endif
