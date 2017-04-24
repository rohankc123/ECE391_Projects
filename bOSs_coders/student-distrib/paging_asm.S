.globl set_cr_reg

set_cr_reg:
  pushl %ebp
  movl %esp, %ebp
  movl 8(%ebp), %ebx
  movl %ebx, %cr3

  movl %cr4, %eax
  or $0x00000010, %eax
  movl %eax, %cr4

  movl %cr0, %eax
  or $0x80000001, %eax
  movl %eax, %cr0
  leave
  ret
