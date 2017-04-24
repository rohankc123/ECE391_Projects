#include "lib.h"
#include "exception_handler.h"
#include "system_call.h"

void exception_0(){
	printf("EXCEPTION: Divide error");
	halt((unsigned char)256);
}

void exception_1(){
	printf("EXCEPTION: RESERVED");
	halt((unsigned char)256);
}

void exception_2(){
	printf("EXCEPTION: NMI Interrupt");
	halt((unsigned char)256);
}

void exception_3(){
	printf("EXCEPTION: Breakpoint");
	halt((unsigned char)256);
}

void exception_4(){
	printf("EXCEPTION: Overflow");
	halt((unsigned char)256);
}

void exception_5(){
	printf("EXCEPTION: BOUND Range Exceeded");
	halt((unsigned char)256);
}

void exception_6(){
	printf("EXCEPTION: Invalid Opcode (Undefined Opcode)");
	halt((unsigned char)256);
}

void exception_7(){
	printf("EXCEPTION: Device Not Available (No Math Coprocessor)");
	halt((unsigned char)256);
}

void exception_8(){
	printf("EXCEPTION: Double Fault");
	halt((unsigned char)256);
}

void exception_9(){
	printf("EXCEPTION: Coprocessor Segment Overrun (reserved)");
	halt((unsigned char)256);
}

void exception_A(){
	printf("EXCEPTION: Invalid TSS");
	halt((unsigned char)256);
}

void exception_B(){
	printf("EXCEPTION: Segment Not Present");
	halt((unsigned char)256);
}

void exception_C(){
	printf("EXCEPTION: Stack-Segment Fault");
	halt((unsigned char)256);
}

void exception_D(){
	printf("EXCEPTION: General Protection");
	halt((unsigned char)256);
}

void exception_E(){
	printf("EXCEPTION: Page Fault\n");
/*
	unsigned long val;
    asm volatile ( "mov %%cr0, %0" : "=r"(val) );
    printf("Invalid Access at: %x\n", val);
 */
	halt((unsigned char)256);
}

void exception_F(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_10(){
	printf("EXCEPTION: x87 FPU Floating-Point Error (Math Fault)");
	halt((unsigned char)256);
}

void exception_11(){
	printf("EXCEPTION: Alignment Check");
	halt((unsigned char)256);
}

void exception_12(){
	printf("EXCEPTION: Machine Check");
	halt((unsigned char)256);
}

void exception_13(){
	printf("EXCEPTION: SIMD Floating-Point Exception");
	halt((unsigned char)256);
}

void exception_14(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_15(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_16(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_17(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_18(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_19(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1A(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1B(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1C(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1D(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1E(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}

void exception_1F(){
	printf("EXCEPTION: INTEL RESERVED");
	halt((unsigned char)256);
}
