# excp_int_sys.S - code for exception handling
# vim:ts=4 noexpandtab

/*
 * From page 158 of https://courses.engr.illinois.edu/ece391/secure/references/IA32-ref-manual-vol-3.pdf
 *
 * When the processor performs a call to the exception- or interrupt-handler procedure:
 * • If the handler procedure is going to be executed at a numerically lower privilege level, a
 * 	stack switch occurs. When the stack switch occurs:
 * 	a. The segment selector and stack pointer for the stack to be used by the handler are
 * 		obtained from the TSS for the currently executing task. On this new stack, the
 * 		processor pushes the stack segment selector and stack pointer of the interrupted
 * 		procedure.
 * 	b. The processor then saves the current state of the EFLAGS, CS, and EIP registers on
 * 		the new stack.
 * 	c. If an exception causes an error code to be saved, it is pushed on the new stack after the
 * 		EIP value.
 * • If the handler procedure is going to be executed at the same privilege level as the
 * 	interrupted procedure:
 * 	a. The processor saves the current state of the EFLAGS, CS, and EIP registers on the
 * 		current stack.
 * 	b. If an exception causes an error code to be saved, it is pushed on the current stack after
 * 		the EIP value.
 */

#define ASM 1

.globl exception0_code
.globl exception1_code
.globl exception2_code
.globl exception3_code
.globl exception4_code
.globl exception5_code
.globl exception6_code
.globl exception7_code
.globl exception8_code
.globl exception9_code
.globl exceptionA_code
.globl exceptionB_code
.globl exceptionC_code
.globl exceptionD_code
.globl exceptionE_code
.globl exceptionF_code
.globl exception10_code
.globl exception11_code
.globl exception12_code
.globl exception13_code
.globl exception14_code
.globl exception15_code
.globl exception16_code
.globl exception17_code
.globl exception18_code
.globl exception19_code
.globl exception1A_code
.globl exception1B_code
.globl exception1C_code
.globl exception1D_code
.globl exception1E_code
.globl exception1F_code
.globl interrupt20_code
.globl interrupt21_code
.globl interrupt22_code
.globl interrupt23_code
.globl interrupt24_code
.globl interrupt25_code
.globl interrupt26_code
.globl interrupt27_code
.globl interrupt28_code
.globl interrupt29_code
.globl interrupt2A_code
.globl interrupt2B_code
.globl interrupt2C_code
.globl interrupt2D_code
.globl interrupt2E_code
.globl interrupt2F_code
.globl syscall_code
.globl prac_sys_call

.align 4

# Divide error
exception0_code:
	pushal
	call exception_0
	popal
	iret

# RESERVED
exception1_code:
	pushal
	call exception_1
	popal
	iret

# NMI Interrupt
exception2_code:
	pushal
	call exception_2
	popal
	iret

# Breakpoint
exception3_code:
	pushal
	call exception_3
	popal
	iret

# Overflow
exception4_code:
	pushal
	call exception_4
	popal
	iret

# BOUND Range Exceeded
exception5_code:
	pushal
	call exception_5
	popal
	iret

# Invalid Opcode (Undefined Opcode)
exception6_code:
	pushal
	call exception_6
	popal
	iret

# Device Not Available (No Math Coprocessor)
exception7_code:
	pushal
	call exception_7
	popal
	iret

# Double Fault
exception8_code:
	pushal
	call exception_8
	popal
	iret

# Coprocessor Segment Overrun (reserved)
exception9_code:
	pushal
	call exception_9
	popal
	iret

# Invalid TSS
exceptionA_code:
	pushal
	call exception_A
	popal
	iret

# Segment Not Present
exceptionB_code:
	pushal
	call exception_B
	popal
	iret

# Stack-Segment Fault
exceptionC_code:
	pushal
	call exception_C
	popal
	iret

# General Protection
exceptionD_code:
	pushal
	call exception_D
	popal
	iret

# Page Fault
exceptionE_code:
	pushal
	call exception_E
	popal
	iret

# Intel reserved. Do not use.
exceptionF_code:
	pushal
	call exception_F
	popal
	iret

# x87 FPU Floating-Point Error (Math Fault)
exception10_code:
	pushal
	call exception_10
	popal
	iret

# Alignment Check
exception11_code:
	pushal
	call exception_11
	popal
	iret

# Machine Check
exception12_code:
	pushal
	call exception_12
	popal
	iret

# SIMD Floating-Point Exception
exception13_code:
	pushal
	call exception_13
	popal
	iret

# Intel reserved. Do not use.
exception14_code:
	pushal
	call exception_14
	popal
	iret

# Intel reserved. Do not use.
exception15_code:
	pushal
	call exception_15
	popal
	iret

# Intel reserved. Do not use.
exception16_code:
	pushal
	call exception_16
	popal
	iret

# Intel reserved. Do not use.
exception17_code:
	pushal
	call exception_17
	popal
	iret

# Intel reserved. Do not use.
exception18_code:
	pushal
	call exception_18
	popal
	iret

# Intel reserved. Do not use.
exception19_code:
	pushal
	call exception_19
	popal
	iret

# Intel reserved. Do not use.
exception1A_code:
	pushal
	call exception_1A
	popal
	iret

# Intel reserved. Do not use.
exception1B_code:
	pushal
	call exception_1B
	popal
	iret

# Intel reserved. Do not use.
exception1C_code:
	pushal
	call exception_1C
	popal
	iret

# Intel reserved. Do not use.
exception1D_code:
	pushal
	call exception_1D
	popal
	iret

# Intel reserved. Do not use.
exception1E_code:
	pushal
	call exception_1E
	popal
	iret

# Intel reserved. Do not use.
exception1F_code:
	pushal
	call exception_1F
	popal
	iret

# pit interrupt
interrupt20_code:
	cli
	pushal
	call scheduler
	popal
	iret

# Keyboard Interrupt
interrupt21_code:
	cli
	pushal
	call keyboard_handler
	popal
	sti
	iret

interrupt22_code:
	iret

interrupt23_code:
	iret

interrupt24_code:
	iret

interrupt25_code:
	iret

interrupt26_code:
	iret

interrupt27_code:
	iret

# RTC
interrupt28_code:
	cli
	pushal
	call rtc_handler
	popal
	sti
	iret

interrupt29_code:
	iret

interrupt2A_code:
	iret

interrupt2B_code:
	iret

interrupt2C_code:
	iret

interrupt2D_code:
	iret

interrupt2E_code:
	iret

interrupt2F_code:
	iret

# System Call
syscall_code:
  pushl %ebp
  movl %esp, %ebp
  # system calls between 1 and 10 check
  cmpl $1, %eax
  jl err

  cmpl $10, %eax
  jg err

  decl %eax

  pushl %esp
  pushl %ebp
  pushl %esi
  pushl %edi
  pushl %edx
  pushl %ecx
  pushl %ebx

  call *jumptable(,%eax,4) # use 4 because address is 32 bits

  popl %ebx
  popl %ecx
  popl %edx
  popl %edi
  popl %esi
  popl %ebp
  popl %esp

  
  leave
  iret

jumptable:
  .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

err:
  xorl %eax, %eax
  decl %eax
  leave
  iret
