#define ASM 1
#include "x86_desc.h"

#define USER_START $0x08000000
#define PROGRAM_SIZE $0x0400000
#define FIRST_ARG 8
.globl entrypoint
.globl sys_halt

entrypoint:
	pushl 	%ebp
	movl 	%esp, %ebp
	xorl 	%eax, %eax
	mov 	$USER_DS, %ax
	mov 	%ax, %ds
	# EIP
	movl 	FIRST_ARG(%ebp), %eax
	# CS
	movl 	$USER_CS, %ebx
	# SS
	movl 	$USER_DS, %ecx

	# esp
	movl 	USER_START, %esi	 # user level esp points to the bottom of program
	addl 	PROGRAM_SIZE, %esi
	subl 	$0x04, %esi # subtract 4 so we are in the correct page
	pushl 	%ecx
	pushl 	%esi
	pushfl
	popl 	%edx
	orl 	$0x200, %edx # interrupt enable flag
	pushl 	%edx
	pushl 	%ebx
	pushl 	%eax

	pushl 	%esp
	pushl 	%ebp
	call  	set_fr
	addl  	$FIRST_ARG, %esp
	iret

halt_done:
	popl	%edx
	popl	%edx
	popl	%edx
	popl	%edx
	popl	%edx

	leave
	ret

sys_halt:
	pushl   %ebp
	movl    %esp, %ebp
	xorl 	%ebx, %ebx
	movl 	FIRST_ARG(%ebp), %ebx

	call    get_pcb
	
	movl    FIRST_ARG(%eax), %eax
	movl    4(%eax), %ebp #get the parent_pcb pointer
	movl    (%eax), %esp

	movl 	%ebx, %eax
	# status
	jmp 	halt_done
