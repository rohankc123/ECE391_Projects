#include "exception_handler.h"
#define ERROR_EXCP 256
void exception_0(){
	printf("EXCEPTION: Divide error");
	real_halt(ERROR_EXCP);
}

void exception_1(){
	printf("EXCEPTION: RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_2(){
	printf("EXCEPTION: NMI Interrupt");
	real_halt(ERROR_EXCP);
}

void exception_3(){
	printf("EXCEPTION: Breakpoint");
	real_halt(ERROR_EXCP);
}

void exception_4(){
	printf("EXCEPTION: Overflow");
	real_halt(ERROR_EXCP);
}

void exception_5(){
	printf("EXCEPTION: BOUND Range Exceeded");
	real_halt(ERROR_EXCP);
}

void exception_6(){
	printf("EXCEPTION: Invalid Opcode (Undefined Opcode)");
	real_halt(ERROR_EXCP);
}

void exception_7(){
	printf("EXCEPTION: Device Not Available (No Math Coprocessor)");
	real_halt(ERROR_EXCP);
}

void exception_8(){
	printf("EXCEPTION: Double Fault");
	real_halt(ERROR_EXCP);
}

void exception_9(){
	printf("EXCEPTION: Coprocessor Segment Overrun (reserved)");
	real_halt(ERROR_EXCP);
}

void exception_A(){
	printf("EXCEPTION: Invalid TSS");
	real_halt(ERROR_EXCP);
}

void exception_B(){
	printf("EXCEPTION: Segment Not Present");
	real_halt(ERROR_EXCP);
}

void exception_C(){
	printf("EXCEPTION: Stack-Segment Fault");
	real_halt(ERROR_EXCP);
}

void exception_D(){
	printf("EXCEPTION: General Protection");
	real_halt(ERROR_EXCP);
}

void exception_E(){
	printf("EXCEPTION: Page Fault");
    real_halt(ERROR_EXCP);
}

void exception_F(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_10(){
	printf("EXCEPTION: x87 FPU Floating-Point Error (Math Fault)");
	real_halt(ERROR_EXCP);
}

void exception_11(){
	printf("EXCEPTION: Alignment Check");
	real_halt(ERROR_EXCP);
}

void exception_12(){
	printf("EXCEPTION: Machine Check");
	real_halt(ERROR_EXCP);
}

void exception_13(){
	printf("EXCEPTION: SIMD Floating-Point Exception");
	real_halt(ERROR_EXCP);
}

void exception_14(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_15(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_16(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_17(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_18(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_19(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1A(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1B(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1C(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1D(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1E(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}

void exception_1F(){
	printf("EXCEPTION: INTEL RESERVED");
	real_halt(ERROR_EXCP);
}
