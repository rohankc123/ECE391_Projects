# pit.S - code for exception handling
# vim:ts=4 noexpandtab
.globl pit_enable

#define ASM 1
#define COUNT $11932
#define CONTROL_WORD $0x34
#define CONTROL_PORT $0x43
#define COUNT_0 $0x40

pit_enable:
	pushl %ebp
	movl %esp, %ebp
	movw COUNT, %dx

	movb CONTROL_WORD, %al
	out %al, CONTROL_PORT

	movw %dx, %ax
	out %al, COUNT_0 # LSB
 	xchg %al, %ah
	out %al, COUNT_0 # MSB

	leave
	ret
