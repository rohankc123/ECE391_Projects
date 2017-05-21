.global start

.text

start:

	movl $1, %eax		#
	movl $1, %edi
	movl $message, %edi	# store 10 in ecx
	syscall

	ret
message:
	.asciz "Hello World"
