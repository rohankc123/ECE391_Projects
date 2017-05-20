.globl set_cr_reg
#define CR4_MASK $0x00000010
#define CR0_MASK $0x80000001
set_cr_reg:
  pushl %ebp
  movl %esp, %ebp
  movl 8(%ebp), %ebx # getting arg
  movl %ebx, %cr3

  movl %cr4, %eax
  or CR4_MASK, %eax # global bit variable
  movl %eax, %cr4

  movl %cr0, %eax
  or CR0_MASK, %eax
  movl %eax, %cr0
  leave
  ret
