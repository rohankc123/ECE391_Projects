.globl rtc_read_wrapper

rtc_write_wrapper:
  pushl %ebp
  movl %esp, %ebp
  pushl %ebx
  call rtc_write
  popl %ebx
  xorl %eax, %eax
  leave
  ret
