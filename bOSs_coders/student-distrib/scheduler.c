#include "scheduler.h"
#include "lib.h"
#include "pcb.h"
#include "paging.h"
#include "i8259.h"
#include "x86_desc.h"
#include "pit_init.h"
#define TERM_MAX 3
int term1_flag = OFF;
int term2_flag = OFF;
int term3_flag = OFF;
int sound_flag = OFF;
//initialize scheduler in reverse order to start at terminal 1
int term = TERM3;

/*
 * Function: scheduler
 * Input:    none
 * Output:   none
 * 		Takes care of scheduling, initializes terminals and switches tasks in a round-robin fashion
 */
void scheduler() {
	uint32_t next_esp = 0;
	uint32_t next_ebp = 0;
	PCB* next_pcb = NULL;
	PCB* cur_pcb = get_pcb();


	if ((term1_flag == ON) || (term2_flag == ON) || (term3_flag == ON)){
		asm __volatile__ ("movl %%esp, %0;"
			:"=r"(cur_pcb->term_esp_reg) /* x is output operand and it's related to %0 */
			:
			:"memory"); /* %eax is clobbered register */

		asm __volatile__ ("movl %%ebp, %0;"
			:"=r"(cur_pcb->term_ebp_reg) /* x is output operand and it's related to %0 */
			:
			:"memory"); /* %eax is clobbered register */
	}

	//initialize terminal 1
	if((term == TERM1) && (term1_flag == OFF)){
		term1_flag = ON;
		term = (term + 1) % TERM_MAX;
		cur_term = TERM1;
		send_eoi(0);
		term_clear(TERM1);
		root_shell(TERM1);
	}

	//initialize terminal 2
	if((term == TERM2) && (term2_flag == OFF)){
		term2_flag = ON;
		term = term - 1;
		cur_term = TERM2;
		send_eoi(0);
		term_clear(TERM2);
		root_shell(TERM2);
	}

	//initialize terminal 3
	if((term == TERM3) && (term3_flag == OFF)){
		term3_flag = ON;
		term = term - 1;
		cur_term = TERM3;
		send_eoi(0);
		term_clear(TERM3);
		root_shell(TERM3);
	}
	//display correct terminal and update cursor
	disp_vmem(cur_term);

	next_pcb = cur_pcb->next;
	while(next_pcb->child_pcb){
		next_pcb = next_pcb->child_pcb;
	}

	next_esp = next_pcb->term_esp_reg;
	next_ebp = next_pcb->term_ebp_reg;
	page_directory.table[PROGRAM_ENTRY] = ((next_pcb->pid*PROGRAM_SIZE+KERNEL_OFFSET)*MB);
	page_directory.present = 1;            // 1 for present
	page_directory.read_write = 1;         // 1 for read/write
	page_directory.user = 1;               // if 1 anyone can access
	page_directory.write_through = 0;      // 1 for write through caching
	page_directory.cache_dis = 0;          // 1 if you don't want caching
	page_directory.accessed = 0;           // 1 if it has been read/written
	page_directory.should_be_zero = 0;
	page_directory.size = 1;               // 1 for 4 mib pages
	page_directory.global = 0;             // ignored

	//update and flush the CR3
	set_table(&page_directory, PROGRAM_ENTRY);
	set_cr_reg(page_directory.table);
	tss.esp0 = KERNEL_OFFSET*MB-(next_pcb->pid*KERNEL_OFFSET*KB)-PAGE_OFF;

	asm __volatile__ ("movl %%ecx, %%ebp;"
	                  "movl %%ebx, %%esp;"
		              :
		              :"c"(next_ebp), "b"(next_esp)
		              :"memory"); /* %eax is clobbered register */

	//switch to next task
	term = (term + 1) % TERM_MAX;
	send_eoi(0);
	return;
}
