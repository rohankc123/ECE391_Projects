#ifndef IDT_HANDLE_H
#define IDT_HANDLE_H

#include "excp_int_sys.h"
#include "exception_handler.h"

//DPL (see set_idt_gate)
#define SUPER_PRIV 	0
#define USER_PRIV 	3

//Size (see set_idt_gate)
#define	ADDR_32		1
#define	ADDR_16		0

#define SEG_DESC_SIZE 8

// number of exceptions and interrupts
#define NUM_INT 48
// number of exceptions
#define NUM_EXCP 32

#define IDT_SIZE 256

extern int init_idt();

void set_idt_gate(uint32_t num, uint32_t offset_31_00, uint16_t seg_selector, uint32_t size, uint32_t dpl, int present_flag);
void set_trap_gate(uint32_t num, uint32_t offset_31_00, uint16_t seg_selector, uint32_t size, uint32_t dpl, int present_flag);

#endif
