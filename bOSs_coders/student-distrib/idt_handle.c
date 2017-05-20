#include "lib.h"

#include "x86_desc.h"
#include "idt_handle.h"

//IDT_SIZE = 256
uint32_t idt_array[IDT_SIZE] = {
	(uint32_t)exception0_code,
	(uint32_t)exception1_code,
	(uint32_t)exception2_code,
	(uint32_t)exception3_code,
	(uint32_t)exception4_code,
	(uint32_t)exception5_code,
	(uint32_t)exception6_code,
	(uint32_t)exception7_code,
	(uint32_t)exception8_code,
	(uint32_t)exception9_code,
	(uint32_t)exceptionA_code,
	(uint32_t)exceptionB_code,
	(uint32_t)exceptionC_code,
	(uint32_t)exceptionD_code,
	(uint32_t)exceptionE_code,
	(uint32_t)exceptionF_code,
	(uint32_t)exception10_code,
	(uint32_t)exception11_code,
	(uint32_t)exception12_code,
	(uint32_t)exception13_code,
	(uint32_t)exception14_code,
	(uint32_t)exception15_code,
	(uint32_t)exception16_code,
	(uint32_t)exception17_code,
	(uint32_t)exception18_code,
	(uint32_t)exception19_code,
	(uint32_t)exception1A_code,
	(uint32_t)exception1B_code,
	(uint32_t)exception1C_code,
	(uint32_t)exception1D_code,
	(uint32_t)exception1E_code,
	(uint32_t)exception1F_code,
	(uint32_t)interrupt20_code,
	(uint32_t)interrupt21_code,
	(uint32_t)interrupt22_code,
	(uint32_t)interrupt23_code,
	(uint32_t)interrupt24_code,
	(uint32_t)interrupt25_code,
	(uint32_t)interrupt26_code,
	(uint32_t)interrupt27_code,
	(uint32_t)interrupt28_code,
	(uint32_t)interrupt29_code,
	(uint32_t)interrupt2A_code,
	(uint32_t)interrupt2B_code,
	(uint32_t)interrupt2C_code,
	(uint32_t)interrupt2D_code,
	(uint32_t)interrupt2E_code,
	(uint32_t)interrupt2F_code
};

/*
 *	init_idt
 * 	DESCRIPTION 	: 	Initializes IDT gates
 *	INPUT		:	NONE
 *	OUTPUT		:	0 			- no errors
 *			:	-1			- error occured
 */
int init_idt(){
	int i;
	//size of idt array
	idt_array[0x80] = (uint32_t)syscall_code;

	//IDT_SIZE = 256
	for (i = 0; i < IDT_SIZE; i++){
		//INTEL RESERVED
		if(i == 15 || (i >= 20 && i < NUM_EXCP)){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 0);
		}
		else if(i < NUM_EXCP){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 1);
		}
		//PICS
		else if(i >= NUM_EXCP){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 0);
		}

		// pit
		if(i == 32){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 1);
		}
		//keyboard interrupt (0x21 IRQ1)
		if(i == 33){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 1);
		}
		//rtc interrupt (0x28 IRQ8)
		if(i == 40){
			set_idt_gate(i, idt_array[i], KERNEL_CS, ADDR_32, SUPER_PRIV, 1);
		}
		//system call
		if(i == 128){
			set_trap_gate(i, idt_array[i], KERNEL_CS, ADDR_32, USER_PRIV, 1);
		}
	}
	return 0;
}

/*
 * set_idt_gate
 * 	DESCRIPTION 	:	Fills out idt_desc_t struct for a given entry
 *	INPUT		:	num			- idt table entry number
 *				offset_31_00 		- offset_15_00  and offset_31_16 entry of idt_desc_t
 *				seg_selector 		- seg_selector entry of idt_desc_t
 *				size 			- size entry of idt_desc_t
 *				dpl 			- dpl entry of idt_desc_t
 *				present_flag		- whether IDT gate is present
 *	OUTPUT		:	NONE
 */
void set_idt_gate(uint32_t num,
		uint32_t offset_31_00,
		uint16_t seg_selector,
		uint32_t size,
		uint32_t dpl,
		int present_flag)
{
	SET_IDT_ENTRY(idt[num], offset_31_00);
	idt[num].seg_selector = seg_selector;
	idt[num].reserved4 = 0;
	idt[num].reserved3 = 0;
	idt[num].reserved2 = 1;
	idt[num].reserved1 = 1;
	idt[num].size = size;
	idt[num].reserved0 = 0;
	idt[num].dpl = dpl;
	idt[num].present = present_flag;
}

/*
 * set_trap_gate
 * 	DESCRIPTION 	:	Fills out idt_desc_t struct for a given entry
 *	INPUT		:	num			- idt table entry number
 *				offset_31_00 		- offset_15_00  and offset_31_16 entry of idt_desc_t
 *				seg_selector 		- seg_selector entry of idt_desc_t
 *				size 			- size entry of idt_desc_t
 *				dpl 			- dpl entry of idt_desc_t
 *				present_flag		- whether IDT gate is present
 *	OUTPUT		:	NONE
 */
void set_trap_gate(uint32_t num,
		uint32_t offset_31_00,
		uint16_t seg_selector,
		uint32_t size,
		uint32_t dpl,
		int present_flag)
{
	SET_IDT_ENTRY(idt[num], offset_31_00);
	idt[num].seg_selector = seg_selector;
	idt[num].reserved4 = 0;
	idt[num].reserved3 = 1;
	idt[num].reserved2 = 1;
	idt[num].reserved1 = 1;
	idt[num].size = size;
	idt[num].reserved0 = 0;
	idt[num].dpl = dpl;
	idt[num].present = present_flag;
}
