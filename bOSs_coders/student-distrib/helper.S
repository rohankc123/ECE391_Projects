#define ASM 1
#include "x86_desc.h"

#define USER_START $0x08000000
#define PROGRAM_SIZE $0x0400000

.globl entrypoint
.globl sys_halt

entrypoint:
	pushl 	%ebp
	movl 	%esp, %ebp
	xorl 	%eax, %eax
	mov 	$USER_DS, %ax
	mov 	%ax, %ds
	# EIP
	movl 	8(%ebp), %eax
	# CS
	movl 	$USER_CS, %ebx
	# SS
	movl 	$USER_DS, %ecx

	# esp
	movl 	USER_START, %esi	 # user level esp points to the bottom of program
	addl 	PROGRAM_SIZE, %esi
	subl 	$0x04, %esi
	pushl 	%ecx
	pushl 	%esi
	pushfl
	popl 	%edx
	orl 	$0x200, %edx
	pushl 	%edx
	pushl 	%ebx
	pushl 	%eax

	pushl 	%esp
	pushl 	%ebp
	call  	set_fr
	addl  	$8, %esp
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
	call    get_pcb

	xorl %ebx, %ebx
	movl 	8(%ebp), %ebx

	movl    8(%eax), %eax
	movl    4(%eax), %ebp
	movl    (%eax), %esp

	movl %ebx, %eax
	# status
	jmp halt_done
