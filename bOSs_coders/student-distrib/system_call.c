#include "lib.h"
#include "system_call.h"
#include "file_system.h"
#include "paging.h"
#include "rtc.h"
#include "pcb.h"
#include "x86_desc.h"
#include "ps2_keyboard.h"
#include "pit_init.h"

#define READ 0
#define WRITE 1
#define OPEN 2
#define CLOSE 3

#define TRUE 1
#define FALSE 0
#define VIDEO 0xB8000
#define PID_SIZE 32

#define KERNEL_OFFSET 8

#define FD_SIZE 7
#define FD_START 2
#define RTC_TYPE 0
#define DIR_TYPE 1
#define FILE_TYPE 2
#define STDIN_TYPE 3
#define STDOUT_TYPE 4

#define STDIN_FD 0
#define STDOUT_FD 1

#define DRIVER 4
#define NUM_DRIVERS 5

#define READ_DATA_BUF 30

#define VID_MEM_PAGE 1023

#define MAGIC_CHAR_0 0x7f
#define MAGIC_CHAR_1 0x45
#define MAGIC_CHAR_2 0x4c
#define MAGIC_CHAR_3 0x46

#define CHECK_0 0
#define CHECK_1 1
#define CHECK_2 2
#define CHECK_3 3
#define ENTRY_IDX 6

#define PROGRAM_VIRT_ADDRESS 0x08048000
#define SCREEN_SHIFT_1 22
#define SCREEN_SHIFT_2 12

uint32_t pid_openings = 0x3F;

/*
 * BACKGROUND INFO
 *
 * Ten system calls, invoked using int $0x80
 * Call number in EAX, first argument in EBX, then ECX, then EDX (max 3 arguments)
 * Should protect all registers from modification by system call to avoid leaking information to user programs
 * Return value placed in EAX if call returns (unless specified, -1 indicates error, 0 indicates successful)
 *
 */

/*
 * Helper function that returns -1 for a read from std_out or write to std_in
 */
int32_t inval_write (int32_t fd, const void* buf, int32_t n_bytes, void* cur_file){
	return -1;
}

int32_t inval_read (int32_t fd, void* buf, int32_t n_bytes, void* cur_file){
	return -1;
}

device rtc_fn;
device dir_fn;
device file_fn;
device stdin_fn;
device stdout_fn;
device ftype_drivers[NUM_DRIVERS];

void system_call(int sys_call_number){
	printf("SYSTEM CALL NUMBER: %d", sys_call_number);
}

void init_callbacks(){
	// Real Time Clock functions:
	rtc_fn.read           = &rtc_read;
	rtc_fn.write          = &rtc_write;
	rtc_fn.open           = &rtc_open;
	rtc_fn.close          = &rtc_close;

	// Directory functions:
	dir_fn.read           = &dir_read;
	dir_fn.write          = &dir_write;
	dir_fn.open           = &dir_open;
	dir_fn.close          = &dir_close;

	// File functions:
	file_fn.read          = &file_read;
	file_fn.write         = &file_write;
	file_fn.open          = &file_open;
	file_fn.close         = &file_close;

	// Standard input output:
	stdin_fn.read         = &terminal_read;
	stdin_fn.write        = &inval_write;
	stdin_fn.open         = &terminal_open;
	stdin_fn.close        = &terminal_close;
	stdout_fn.read        = &inval_read;
	stdout_fn.write       = &terminal_write;
	stdout_fn.open        = &terminal_open;
	stdout_fn.close       = &terminal_close;

	ftype_drivers[RTC_TYPE]      = rtc_fn;
	ftype_drivers[DIR_TYPE]      = dir_fn;
	ftype_drivers[FILE_TYPE]     = file_fn;
	ftype_drivers[STDIN_TYPE]    = stdin_fn;
	ftype_drivers[STDOUT_TYPE]   = stdout_fn;
}


void set_fr(uint32_t _ebp, uint32_t _esp);
void set_term_fr(uint32_t _ebp, uint32_t _esp);
void fetch_fr(uint32_t _ebp, uint32_t _esp);

//-----------------------------------------------------------------------------
// REAL_HALT
//     This system call is called when a program is terminating and cleans up the
//     stack and PCB in lieu of the terminated program.
//
//-----------------------------------------------------------------------------
int32_t real_halt(int32_t status)
{
	cli();
	int i, pid;
	PCB* child_pcb = get_pcb();
	PCB* parent_pcb = child_pcb->parent_pcb;

	set_buf_end(0); //Sets the terminal buffer to empty (start=0)

	// Close all open files
	for (i = FD_START; i < FD_NUM; ++i){
		close(i);
	}

	// Free the child processes page from memory
	pid = child_pcb->pid;
	pid_openings |= 1 << pid;

	// Re-start shell if the parent is NULL:
	if(parent_pcb == NULL){
		term_clear(child_pcb->term_parent);
		root_shell(child_pcb->term_parent);
		return 0;
	}
	parent_pcb->child_pcb = NULL;

	// Set up a single 4 MB page directory entry that maps virtual address
	// 0x08000000 (128 MB) to the right physical memory address (either 8 MB or 12 MB)
	page_directory.table[PROGRAM_ENTRY] = ((parent_pcb->pid*PROGRAM_SIZE+KERNEL_OFFSET)*MB);
	page_directory.present = 1;            // 1 for present
	page_directory.read_write = 1;         // 1 for read/write
	page_directory.user = 1;               // if 1 anyone can access
	page_directory.write_through = 0;      // 1 for write through caching
	page_directory.cache_dis = 0;          // 1 if you don't want caching
	page_directory.accessed = 0;           // 1 if it has been read/written
	page_directory.should_be_zero = 0;
	page_directory.size = 1;               // 1 for 4 mib pages
	page_directory.global = 0;             // ignored

	// Update and flush the CR3
	set_table(&page_directory, PROGRAM_ENTRY);
	set_cr_reg(page_directory.table);

	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERNEL_OFFSET*MB-(parent_pcb->pid*KERNEL_OFFSET*KB)-PAGE_OFF;
	sys_halt(status);

	return -1;
}

//-----------------------------------------------------------------------------
// Syscall HALT
//     wrapper for real_halt function only gets the bottom 8 bits of our argument
//     for security purposes
//
//-----------------------------------------------------------------------------

int32_t halt (uint8_t status){
	real_halt((int32_t)status);
	return -1;
}

//-----------------------------------------------------------------------------
// root shell
//     This system call is called when a user wishes to execute a "root" shell
//     which has no parent pointers
//     It copies the program into memory, sets up a stack and makes a new
//     PCB for the program.
//
//-----------------------------------------------------------------------------
int32_t root_shell(int terminal){
	cli();
	uint8_t command[] = "shell";
	dentry_t dentry;
	dentry_t* temp = &dentry;
	uint8_t* fname;
	int i, pid_num;
	for(i = 0; i < PID_SIZE; i++){
		/* check if pid i is open */
		if((pid_openings >> i) & 1){
			break;
		}
	}

	/* If there are no PID openings */
	if(i == PID_SIZE) return -1;
	pid_num = i;
	pid_num = terminal;
	i = terminal;

	//8 kilobyte mask
	PCB* child_pcb = (PCB*)((KERNEL_OFFSET*MB-(KERNEL_OFFSET*KB*terminal)-PAGE_OFF) & PCB_MASK);
	child_pcb->pid = i;
	uint32_t entry_esp;

	fname = command;

	for (i = 0; i < KBD_BUF_SIZE-1; i++ ){
		child_pcb->args_buf[i] = 0;
	}

	//fill dentry structure
	if(read_dentry_by_name(fname, temp) == -1){
		return -1;
	}

	uint8_t buf[READ_DATA_BUF];
	if(read_data(temp->inode, 0, buf, READ_DATA_BUF) == -1){
		return -1;
	}

	//if not exe, fail
	/* if magic number is not present, the execute system call should fail */
	/* magic number is first 4 bytes of the file- */
	/* 0: 0x7f, 1: 0x45, 2: 0x4c, 3: 0x46 */
	if(!(buf[CHECK_0] == MAGIC_CHAR_0 && buf[CHECK_1] == MAGIC_CHAR_1 && buf[CHECK_2] == MAGIC_CHAR_2 && buf[CHECK_3] == MAGIC_CHAR_3)){
		return -1;
	}

	/* Set the PCB values */
	child_pcb->next = (PCB*)((KERNEL_OFFSET*MB-(KERNEL_OFFSET*KB*((terminal+1)%CHECK_3))-PAGE_OFF) & PCB_MASK); //get setup the next address 3 is max num term
	child_pcb->parent_pcb = NULL ;//parent_pcb;
	child_pcb->child_pcb = NULL;
	(child_pcb->open_files[STDIN_FD]).table_pointer = ftype_drivers[STDIN_TYPE];
	(child_pcb->open_files[STDOUT_FD]).table_pointer = ftype_drivers[STDOUT_TYPE];
	(child_pcb->open_files[STDIN_FD]).flags = 1;
	(child_pcb->open_files[STDOUT_FD]).flags = 1;

	child_pcb->term_parent = terminal;

	pid_openings &= ~(0x0001 << pid_num);

	/* Clear fds */
	for(i = FD_START; i < FD_NUM; i++){
		(child_pcb->open_files[i]).flags = 0;
	}

	//set up a single 4 MB page directory entry that maps virtual address 0x08000000 (128 MB) to the right physical memory address (either 8 MB or 12 MB)
	page_directory.table[PROGRAM_ENTRY] = ((child_pcb->pid*PROGRAM_SIZE+KERNEL_OFFSET)*MB);
	page_directory.present = 1;  // 1 for present
	page_directory.read_write = 1; //1 for read/write
	page_directory.user = 1; //if 1 anyone can access
	page_directory.write_through = 0; //1 for write through caching
	page_directory.cache_dis = 0; //1 if you don't want caching
	page_directory.accessed = 0; //1 if it has been read/written
	page_directory.should_be_zero = 0;
	page_directory.size = 1; //1 for 4 mib pages
	page_directory.global = 0; //ignored

	set_table(&page_directory, PROGRAM_ENTRY);
	set_cr_reg(page_directory.table);

	//then, program image must be copied to correct offset (0x00048000) within that page
	uint8_t* new_program = (uint8_t*)PROGRAM_VIRT_ADDRESS;
	read_data(temp->inode, 0, new_program, temp->fsize);
	tss.ss0 = KERNEL_DS;
	/* Kernel stack goes at bottom of kernel memory offset up by 8KB * pid */
	tss.esp0 = KERNEL_OFFSET*MB-(child_pcb->pid*KERNEL_OFFSET*KB)-PAGE_OFF;
	//jump to entry point of program to begin execution should be bytes 24-27
	entry_esp = ((uint32_t*)buf)[ENTRY_IDX];
	entrypoint(entry_esp);

	//return 256 if program dies by exception
	//returns value in range 0 to 255 if the program executes a halt system call, in which the value
	//returned is that given by the program's call to halt

	return 0;
}

//-----------------------------------------------------------------------------
// Syscall EXECUTE, #2
//     This system call is called when a program wishes to write data to
//     the file system.
//
//     Input:        	command - file name
//     Return:		halt number
//
//-----------------------------------------------------------------------------
int32_t execute (const uint8_t* command){
	if(!command) return -1;
	cli();
	dentry_t dentry;
	dentry_t* temp = &dentry;
	uint8_t fname[NAME_LENGTH];
	PCB* parent_pcb = get_pcb();
	int i;
	int j;
	int pid_num;
	int leading_spaces = FALSE;
	//start at 3 bc previous numbers are reserved for terminals
	for(i = 3; i < PID_SIZE; i++){
		/* check if pid i is open */
		if((pid_openings >> i) & 1){
			break;
		}
	}
	/* If there are no PID openings */
	if(i == PID_SIZE){
		return -1;
	}
	pid_num = i;

	// top 8 kb
	PCB* child_pcb =  (PCB*)((KERNEL_OFFSET*MB-(KERNEL_OFFSET*KB*i)-PAGE_OFF) & PCB_MASK);

	child_pcb->pid = i;
	uint32_t entry_esp;

	//extract name from command ignoring initial whitespace
	for(i = 0; i < NAME_LENGTH; i++){
		if(command[i] != ' ' ){
			fname[i] = command[i];
		}
		else break;
		if(command[i] == '\0') break;
	}
	fname[i] = '\0';

	/* The -1 is because KBD_BUF_SIZE includes space for a new line carriage which we don't want */
	for(j = i; j < KBD_BUF_SIZE+i-1; j++){
		/* increment past spaces */
		if(command[j] == ' ' && !leading_spaces){
			i++;
			continue;
		}
		leading_spaces = TRUE;
		child_pcb->args_buf[j-i] = command[j];
		if(command[j] == '\0'){
			break;
		}
	}
		child_pcb->args_buf[j-i] = '\0';
	//fill dentry structure
	if(read_dentry_by_name(fname, temp) == -1){
			return -1;
	}

	uint8_t buf[READ_DATA_BUF]; //4 Bytes to read
	if(read_data(temp->inode, 0, buf, READ_DATA_BUF) == -1){ //4 Bytes to read
		return -1;
	}

	/*if not exe, fail
	 * if magic number is not present, the execute system call should fail
	 * magic number is first 4 bytes of the file
	 * 0: 0x7f, 1: 0x45, 2: 0x4c, 3: 0x46 						*/
	 if(!(buf[CHECK_0] == MAGIC_CHAR_0 && buf[CHECK_1] == MAGIC_CHAR_1 && buf[CHECK_2] == MAGIC_CHAR_2 && buf[CHECK_3] == MAGIC_CHAR_3)){
 		return -1;
 	}

	/* Set the PCB values */
	child_pcb->parent_pcb = parent_pcb;
	child_pcb->next = child_pcb->parent_pcb->next;
	child_pcb->child_pcb = NULL;
	(child_pcb->open_files[STDIN_FD]).table_pointer = ftype_drivers[STDIN_TYPE];
	(child_pcb->open_files[STDOUT_FD]).table_pointer = ftype_drivers[STDOUT_TYPE];
	(child_pcb->open_files[STDIN_FD]).flags = 1;
	(child_pcb->open_files[STDOUT_FD]).flags = 1;


	child_pcb->term_parent = child_pcb->parent_pcb->term_parent;

	pid_openings &= ~(0x0001 << pid_num);
	parent_pcb->child_pcb = child_pcb;
	child_pcb->parent_pcb = parent_pcb;

	/* Clear fds */
	for(i = FD_START; i < FD_NUM; i++){
		(child_pcb->open_files[i]).flags = 0;
	}

	//set up a single 4 MB page directory entry that maps virtual address 0x08000000 (128 MB) to the right physical memory address (either 8 MB or 12 MB)
	page_directory.table[PROGRAM_ENTRY] = ((child_pcb->pid*PROGRAM_SIZE+KERNEL_OFFSET)*MB);
	page_directory.present = 1;  // 1 for present
	page_directory.read_write = 1; //1 for read/write
	page_directory.user = 1; //if 1 anyone can access
	page_directory.write_through = 0; //1 for write through caching
	page_directory.cache_dis = 0; //1 if you don't want caching
	page_directory.accessed = 0; //1 if it has been read/written
	page_directory.should_be_zero = 0;
	page_directory.size = 1; //1 for 4 mib pages
	page_directory.global = 0; //ignored

	set_table(&page_directory, PROGRAM_ENTRY);
	set_cr_reg(page_directory.table);

	//then, program image must be copied to correct offset (0x00048000) within that page
	uint8_t* new_program = (uint8_t*)PROGRAM_VIRT_ADDRESS;
	read_data(temp->inode, 0, new_program, temp->fsize);
	tss.ss0 = KERNEL_DS;
	/* Kernel stack goes at bottom of kernel memory offset up by 8KB * pid */
	tss.esp0 = KERNEL_OFFSET*MB-(child_pcb->pid*KERNEL_OFFSET*KB)-PAGE_OFF;
	//jump to entry point of program to begin execution should be bytes 24-27
	entry_esp = ((uint32_t*)buf)[ENTRY_IDX];
	uint32_t result;
	result = entrypoint(entry_esp);

	//return 256 if program dies by exception
	//returns value in range 0 to 255 if the program executes a halt system call, in which the value
	//returned is that given by the program's call to halt

	return result;
}

//-----------------------------------------------------------------------------
// Syscall READ, #3
//     This system call is called when a program wishes to access read from
//     the file system.
//
//     Input:        fd - File Descriptor
//                  buf - Buffer to be written to
//               nbytes - The number of bytes requested
//     Output:            The number of bytes written
//
//-----------------------------------------------------------------------------
int32_t read (int32_t fd, void* buf, int32_t nbytes){
	if(!buf) return -1;
	/* Check for invalid index */
	sti();

	if (fd < 0 || fd > FD_SIZE)
	    return -1;

	PCB* cur_pcb = get_pcb();

	/* Check for invalid fd */
	if(cur_pcb->open_files[fd].flags == 0)
	    return -1;

	return (cur_pcb->open_files[fd].table_pointer.read(fd, (uint8_t*)buf, nbytes, &(cur_pcb->open_files[fd])));
}

//-----------------------------------------------------------------------------
// Syscall WRITE, #4
//     This system call is called when a program wishes to write data to
//     the file system.
//
//     Input:        fd - File Descriptor
//                  buf - Buffer to be written to
//               nbytes - The number of bytes requested
//     Output:            The number of bytes written
//
//-----------------------------------------------------------------------------
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
	if(!buf) return -1;
	/* Check for invalid index */
	if (fd < 0 || fd > FD_SIZE)
		return -1;

	PCB* cur_pcb = get_pcb();
	/* Check for invalid fd */
	if(cur_pcb->open_files[fd].flags == 0){
		return -1;
	}

	return cur_pcb->open_files[fd].table_pointer.write(fd, (uint8_t*)buf, nbytes, NULL);
}

//-----------------------------------------------------------------------------
// Syscall OPEN, #5
//     This system call is called when a program wishes to open a file.
//
//     Input:  filename - name of file to be opened
//     Output:        0 - success
//                   -1 - error
//
//-----------------------------------------------------------------------------
int32_t open (const uint8_t* filename){
	if(!filename) return -1;
	dentry_t cur_dentry; //dentry associated with fd
	const uint8_t* temp_name = filename;
	int i;
	if(read_dentry_by_name((uint8_t*)temp_name, &cur_dentry)==-1){
		//Bad fd error
		return -1;
	}

	PCB* cur_pcb = get_pcb();
	for(i = FD_START; i < (FD_SIZE+1); i++){
		if((cur_pcb->open_files[i]).flags == 0){
			break;
		}
	}

	/* No open file descriptors */
	if(i == FD_SIZE+1){
		return -1;
	}

	open_file* newfile = &(cur_pcb->open_files[i]);

	newfile->table_pointer =  ftype_drivers[cur_dentry.ftype];
	newfile->inode = cur_dentry.inode;
	newfile->filepos = 0;
	newfile->flags = 1;

	return i;
}

//-----------------------------------------------------------------------------
// Syscall CLOSE, #6
//     This system call is called when a program wishes to close a file.
//
//     Input:        fd - file descriptor of an open file
//     Output:        0 - success
//                   -1 - error
//
//-----------------------------------------------------------------------------
int32_t close (int32_t fd){
	if(fd < FD_START || fd > FD_SIZE) return -1;

	PCB* cur_pcb = get_pcb();

	if((cur_pcb->open_files[fd]).flags == 0) return -1;
	(cur_pcb->open_files[fd]).flags = 0;
	(cur_pcb->open_files[fd]).filepos = 0;
	return 0;
}

//-----------------------------------------------------------------------------
// Syscall GETARGS, #7
//     Writes the current PCB arguments into the buffer.
//
//     Input:       buf - Buffer to be written to
//               nbytes - Number of bytes requested
//     Output:        0 - success
//                   -1 - error
//
//-----------------------------------------------------------------------------
int32_t getargs (uint8_t* buf, int32_t nbytes){
	if(!buf) return -1;
	int i;
	int full_arg_buf = FALSE;

	PCB* cur_pcb = get_pcb();

	for(i = 0; i < nbytes; i++){
		buf[i] = cur_pcb->args_buf[i];
		if(cur_pcb->args_buf[i] == '\0'){
			full_arg_buf = TRUE;
			break;
		}
	}
	if(i == 0) return -1;
	if(full_arg_buf) return 0;

	/* if buf isn't big enough for the whole argument buffer */
	return -1;
}

//-----------------------------------------------------------------------------
// Syscall VIDMAP, #8
//     returns through the parameters a pointer to the start of a 4kB page
//     that is mapped to video memory. (NOT ok to change permission of video
//     memory that is <4MB and return the address).
//
//     Input:  sreen_start - location of video mem.
//     Output:           0 - success
//                      -1 - error
//
//-----------------------------------------------------------------------------
int32_t vidmap (uint8_t** screen_start){
	if(!screen_start) return -1;

	/* Test for invalid memory address */
	if((uint32_t)screen_start == (uint32_t)0x0 || (uint32_t)screen_start < (uint32_t)KERNEL_OFFSET	*MB) return -1;

	//unsigned int index = next_vidmap();
	PCB* process = get_pcb();

	//video memory
	video_page_table.page[process->pid] = (uint32_t)video_copiez[process->term_parent];
	video_page_table.present = 1;
	video_page_table.read_write = 1; //1 for read/write
	video_page_table.user = 1; //if 1 anyone can access
	video_page_table.write_through = 0; //1 for write through caching
	video_page_table.cache_dis = 0; //1 if you don't want caching
	video_page_table.accessed = 0; //1 if it has been read/written
	video_page_table.dirty = 0; //1 if it has been written to
	video_page_table.should_be_zero = 0;
	video_page_table.global = 1; //if set prevents tlb from updating the address if cr3 is reset
	set_page(&video_page_table, process->pid);

	*screen_start = (uint8_t*)(0x00000000 | (VID_MEM_PAGE << SCREEN_SHIFT_1) | (process->pid << SCREEN_SHIFT_2));

	return 0;
}

//-----------------------------------------------------------------------------
// Syscall SET_HANDLER, #9
//
//     Input:             signum
//               handler_address
//     Output:                 0 - success
//                            -1 - error
//
//-----------------------------------------------------------------------------
int32_t set_handler (int32_t signum, void* handler_address){
	return -1;
}

//-----------------------------------------------------------------------------
// Syscall SIGRETURN, #10
//
//     Input:
//     Output:        0 - success
//                   -1 - error
//
//-----------------------------------------------------------------------------
int32_t sigreturn (void){
	return -1;
}

// set_frame_regs
//    This function stores the ESP and EBP of a parent process into the PCB
//-----------------------------------------------------------------------------
void set_fr(uint32_t _ebp, uint32_t _esp) {
	PCB* this_pcb = get_pcb();
	this_pcb->esp_reg = _esp;
	this_pcb->ebp_reg = _ebp;
	return;
}

// fetch_frame_regs
//     This function retrieves the values of the ESP and EBP of a parent process an
//     stores them into the args.
//-----------------------------------------------------------------------------
void fetch_fr(uint32_t _ebp, uint32_t _esp) {
	PCB* this_pcb = get_pcb();
	_esp = this_pcb->esp_reg;
	_ebp = this_pcb->ebp_reg;
	return;
}
