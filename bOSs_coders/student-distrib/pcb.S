# pcb.S - code for getting the top of pcb
# vim:ts=4 noexpandtab
#define KB_8_MASK $0xFFFFE000

#define ASM 1

.globl get_pcb

get_pcb:
  push %ebp
  movl %esp, %ebp
  movl %esp, %eax
  andl KB_8_MASK, %eax
  leave
  ret
