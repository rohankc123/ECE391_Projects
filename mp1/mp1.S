
.data					# section declaration

        # Useful offset constants for accessing members of a
        # struct mp1_blink_struct structure
        LOCATION   = 0
        ON_CHAR    = 2
        OFF_CHAR   = 3
        ON_LENGTH  = 4
        OFF_LENGTH = 6
        COUNTDOWN  = 8
        STATUS     = 10
        NEXT       = 12

        STRUCT_SIZE = 16

# Pointer to head of list (initialized to NULL)
mp1_list_head:
        .long   0

.text					# section declaration

# Export the function symbol names

.global mp1_rtc_tasklet
.global mp1_ioctl

# void mp1_poke(void);
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX
mp1_poke:

	movl    vmem_base_addr(,1),%edx
	movb    %cl,(%edx,%eax,1)
	ret

# void mp1_rtc_tasklet(unsigned long);
#
# Registers:	Clobbers EDX
#				Clobbers EAX
#				Clobbers CL
# Local Var:	%eax - LOCATION
#				%edx - LL head
#				%ebx - COUNTDOWN
#				%edi - STATUS
#				%esi - ON/OFF_CHAR
#				%cl  - byte sent to poke
mp1_rtc_tasklet:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	pushl %ebx 					# save ebx for user
	pushl %edi					# save edi for user
	pushl %esi 					# save esi for user
	xorl %edx, %edx				# initialize edx
	movl (mp1_list_head), %edx 	# initialize edx as linked-list head
ll_search_loop:
	cmpl $0, %edx	 			# see if head == NULL
	je rtc_tasklet_exit			# jump to function exit
	movw COUNTDOWN(%edx), %bx 	# store COUNTDOWN in ebx
	subl $1, %ebx				# decrement COUNTDOWN
	movw %bx, COUNTDOWN(%edx)	# store back into COUNTDOWN
	cmpw $0, %bx  				# check if COUNTDOWN is 0
	jg no_countdown				# jump to no_countdown if not 0
	movw STATUS(%edx), %di	 	# store status in %edi
	not %di 					# toggle edi
	and $0x1, %edi				# mask edi with 0000 0001
	movw %di, STATUS(%edx)		# store status back
	testw %di, %di 				# test if off_char or on_char
	je show_off_char			# if new status code is 0, display off char
show_on_char:
	movb ON_CHAR(%edx), %cl 	# move ON_CHAR to cl
	movw ON_LENGTH(%edx), %bx 	# set new COUNTDOWN
	jmp char_loaded 			# jump past OFF_CHAR
show_off_char:
	movb OFF_CHAR(%edx), %cl 	# move OFF_CHAR to cl
	movw OFF_LENGTH(%edx), %bx 	# set new COUNTDOWN
char_loaded:
	xorl %eax, %eax				# clear to move location for poke
	movw LOCATION(%edx), %ax 	# move LOCATION to eax
	shlw $1, %ax				# shift LOCATION left 1
	pushl %edx 					# push edx for mp1_poke call
	call mp1_poke 				# call poke function
	popl %edx 					# pop edx back from stack
	movw %bx, COUNTDOWN(%edx)	# set new COUNTDOWN value
no_countdown:
	movl NEXT(%edx), %edx 		# set new LL head regardless of countdown
	jmp ll_search_loop 			# return to top of loop
rtc_tasklet_exit:
	popl %esi					# restore caller registers
	popl %edi
	popl %ebx
	leave
	ret

# Dispatcher
#
# Registers:	Clobbers ECX
#				Clobbers EAX
#
# Local Vars:	ECX - cmd
mp1_ioctl:
	movl 8(%esp), %ecx 			# cmd into ecx
	cmpl $4, %ecx 				# for cmd errors > 3
	jae cmd_error				#
	jmp *jump_table(, %ecx, 4)	# use jump table
cmd_error:
	movl $-1, %eax				# return value -1
	ret

# Add a node to the head of the ll
#
# Registers:	Clobbers EDX
#				Clobbers EAX
#				Clobbers ECX
#
# Local Vars: 	ESI - new node
#				EDX - LL head
# 				ECX - arg (new struct ptr)
# 				EAX - func returns
mp1_ioctl_add:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	movl 8(%ebp), %ecx			# store user struct ptr in %ecx
	testl %ecx, %ecx 			# test if address is NULL
	je error_find_location		# then return error
	pushl %esi 					# save callee registers
	movl mp1_list_head, %edx 	# edx holds ll head
	pushl $STRUCT_SIZE 			# malloc STRUCT_SIZE
	call mp1_malloc				#
	testw %ax, %ax 				# test for error
	je error_free_mem 			# ERROR
	movl 8(%ebp), %ecx			# store user struct ptr in %ecx
	movl %eax, %esi				# set esi to new memory
	pushl %esi 					# save esi
	pushl $STRUCT_SIZE 			# prep mp1_copy_from_user, n
	pushl %ecx 					# from
	pushl %esi 					# to
	call mp1_copy_from_user 	# CALL
	addl $12, %esp 				# pop args
	popl %esi 					# restore esi
	testw %ax, %ax 				# test for error
	jne error_free_mem 			# ERROR
	movw LOCATION(%esi), %ax 	# check location
	cmpw $2000, %ax			 	# test for valid location, (less than 2000)
	jae error_free_mem 			# ERROR
	movl (mp1_list_head), %edx 	# restore edx
	movl %edx, NEXT(%esi)		# set new struct data
	movw ON_LENGTH(%esi), %ax	#
	movw %ax, COUNTDOWN(%esi) 	#
	movw $1, STATUS(%esi) 		#
	movl %esi, (mp1_list_head) 	# Set as new head
	movw LOCATION(%esi), %ax 	# prep for poke
	shlw $1, %ax				# shift LOCATION left 1
	movb ON_CHAR(%esi), %cl 	#
	call mp1_poke 				# CALL
	popl %esi					# resotre caller register
	leave
	ret
error_free_mem:
	pushl %eax 					# push for memory free
	call mp1_free 				# mem free
	movl $-1, %eax 				# return error
	popl %ecx 					# restore callee registers
	popl %esi					#
	leave
	ret

# Remove node specified by arg location
#
# Registers:	clobbers EAX
#				clobbers ECX
#				clobbers EDX
#
# Local Vars:	EAX - node pointing to node to be removed
#				EDX - node to be removed
mp1_ioctl_remove:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	movl 8(%ebp), %edx			# store location remove in edx
	call mp1_ll_traverse		# call ll search function
	testw %ax, %ax	 			# if there is no matching location
	je error_find_location 		# if eax is NULL
	testl %ecx, %ecx 			# test if eax is head
	je change_head_remove 		#
	pushl %eax 					# prep for free
	movl NEXT(%eax), %edx 		#
	movl %edx, NEXT(%ecx)		#
	call mp1_free 				# free memory
	movl $0, %eax				# standard return
	leave						#
	ret							#
change_head_remove:
	pushl %eax 					# prep for free
	movl NEXT(%eax), %edx	 	# store new head
	movl %edx, mp1_list_head	#
	call mp1_free 				# free memory
	movl $0, %eax				# standard return
	leave						#
	ret							#

# Find node specified by location and return it to user
#
# Registers:	clobbers: EAX
#				clobbers: ECX
#
# Local Vars:
mp1_ioctl_find:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	movl 8(%ebp), %ecx			# store arg in ecx
	movw LOCATION(%ecx), %dx 	# prep for ll_traverse
	cmpw $2000, %dx			 	# test for valid location, (less than 2000)
	jae error_find_location 	# return ERROR
	pushl %ecx 					# store ecx
	call mp1_ll_traverse		# search for element
	popl %ecx 					# restore ecx
	testl %eax, %eax 			# test for NULL location
	je error_find_location 		# return ERROR
	pushl $STRUCT_SIZE 			# prep for copy_to_user
	pushl %eax					#
	pushl %ecx 					#
	call mp1_copy_to_user		# call
	movl $0, %eax 				# normal return
	leave						#
	ret							#

# Sync two nodes to each other
#
# Registers:	Clobbers EDX
#				Clobbers ECX
#				Clobbers EAX
#
# Local Vars:	EDI - Holds first argument
# 				ESI - Holds second argument
# 				ECX - Holds first struct
# 				EDX - Holds second struct
#				CL  - Poke
mp1_ioctl_sync:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	pushl %edi					# save caller registers
	pushl %esi					#
	movl 8(%ebp), %edi			# store arg in edi
	movl %edi, %esi 			# prep two arguments
	shrl $16, %esi 				#
	cmpw $2000, %si			 	# test for valid location, (less than 2000)
	jae error_find_location 	# return ERROR
	cmpw $2000, %di			 	# test for valid location, (less than 2000)
	jae error_find_location 	# return ERROR
	movw %di, %dx 				# prep for ll_traverse
	call mp1_ll_traverse 		# call
	testl %eax, %eax 			# test for NULL address
	je error_find_location_sync # return ERROR
	movl %eax, %ecx 			# store first struct
	movw %si, %dx 				# prep for ll_traverse
	pushl %ecx
	call mp1_ll_traverse 		# call
	popl %ecx
	testl %eax, %eax 			# test for NULL address
	je error_find_location_sync # return ERROR
	movl %eax, %edx 			# store second struct
	xorl %eax, %eax 			# Move first struct to second
	movw ON_LENGTH(%edx), %ax 	#
	movw %ax, ON_LENGTH(%ecx) 	#
	xorl %eax, %eax 			#
	movw OFF_LENGTH(%edx), %ax 	#
 	movw %ax, OFF_LENGTH(%ecx)	#
	xorl %eax, %eax 			#
	movw COUNTDOWN(%edx), %ax 	#
	movw %ax,  COUNTDOWN(%ecx)	#
	xorl %eax, %eax 			#
	movw STATUS(%edx), %ax 		#
	movw %ax, STATUS(%ecx)		#
	testw %ax, %ax				# test if off_char or on_char
	je show_off_char_sync		# if new status code is 0, display off char
show_on_char_sync:
	movb ON_CHAR(%ecx), %cl 	# move ON_CHAR to cl
	jmp char_loaded_sync		# jump past OFF_CHAR
show_off_char_sync:
	movb OFF_CHAR(%ecx), %cl 	# move OFF_CHAR to cl
char_loaded_sync:
	xorl %eax, %eax 			#
	movw LOCATION(%ecx), %ax	# prep for poke
	shlw $1, %ax				# shift LOCATION left 1
	call mp1_poke 				# call
	popl %esi					# restore caller registers
	popl %edi					#
	leave 						#
	ret							#
error_find_location_sync:
	popl %esi					# restore caller registers
	popl %edi					#
	movl $0, %eax 				# return ERROR
	leave
	ret

# Traverse linked list to find a node location
#
# Interface: 	Register-based arguments
# 	Inputs: 	DX - location of ll element to find
#
#	Outputs: 	EAX - ll element with location or NULL if EDX location not found
#				ECX - ll element before EAX, NULL if first element contains location
# Registers: 	Clobbers EAX
#				Clobbers ECX
mp1_ll_traverse:
	pushl %ebp					# store old stack base pointer
	movl %esp, %ebp 			# set esp to new ebp
	pushl %ebx					# store caller register
	movl mp1_list_head, %eax 	# get list start
	xorl %ecx, %ecx 			# initialize as NULL
ll_traverse_loop:
	testl %eax, %eax			# test if head is NULL
	je next_found 				# exit if it is
	movw LOCATION(%eax), %bx 	# move current location into bx
	cmpw %bx, %dx				# test if cur location is dx
	je next_found 				# return if it is found
	movl %eax, %ecx				# holds previous reg
	movl NEXT(%eax), %eax 		# next list element
	jmp ll_traverse_loop		#
next_found:
	popl %ebx 					# restore caller register
	leave						#
	ret							#

# Jump area for errors in functions above
error_find_location:
	movl $-1, %eax 				# return -1 for error
	leave						#
	ret							#

jump_table:
.long mp1_ioctl_add, mp1_ioctl_remove, mp1_ioctl_find, mp1_ioctl_sync
.end
