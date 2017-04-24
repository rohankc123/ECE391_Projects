/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "idt_handle.h"
#include "paging.h"
#include "file_system.h"
#include "excp_int_sys.h"
#include "x86_desc.h"
#include "system_call.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;
		file_system_addr = (unsigned int)mod->mod_start;
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000;
		ltr(KERNEL_TSS);
	}

	clear();
	if(init_idt() == -1); //TODO: Add error handling

	/* Init the PIC */
	i8259_init();

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */
	initialize_kb();
	rtc_init();
	paging_init();

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */
	/* printf("Enabling Interrupts\n"); */
	sti();

/************************************ test index ************************************
	clear();
	uint32_t i;
	char typec[8];
	char sizec[32];
	dentry_t dentry;
	dentry_t* temp = &dentry;

	uint8_t name[6] = "name: ";
	uint8_t type[18] = "            type: ";
	uint8_t size[7] = " size: ";

	for(i = 0; i < 17; i++){
		read_dentry_by_index(i, temp);

		terminal_write((char*)name, 6);
		terminal_write((char*)temp->fname, 32);

		terminal_write((char*)type, 18);
		memset(&typec[0], 0, sizeof(typec));
		itoa(temp->ftype, typec, 8);
		terminal_write(typec, 8);

		terminal_write((char*)size, 7);
		memset(&sizec[0], 0, sizeof(sizec));
		itoa(temp->fsize, sizec, 10);
		terminal_write(sizec, 9);
	}
*/


/************************************ test name ************************************
	clear();
	char typec[8];
	char sizec[32];
	dentry_t dentry;
	dentry_t* temp = &dentry;

	uint8_t fname[] = ".";
	uint8_t name[] = "name: ";
	uint8_t type[] = "            type: ";
	uint8_t size[] = " size: ";

	read_dentry_by_name(fname, temp);

	terminal_write((char*)name, 6);
	terminal_write((char*)temp->fname, 32);

	terminal_write((char*)type, 18);
	memset(&typec[0], 0, sizeof(typec));
	itoa(temp->ftype, typec, 8);
	terminal_write(typec, 8);

	terminal_write((char*)size, 7);
	memset(&sizec[0], 0, sizeof(sizec));
	itoa(temp->fsize, sizec, 10);
	terminal_write(sizec, 9);
 */

/************************************ test read ************************************
	clear();
	dentry_t dentry;
	dentry_t* temp = &dentry;

	uint8_t fname[] = "shell";

	found = read_dentry_by_name(fname, temp);

	uint8_t buf[temp->fsize];
	read_data(temp->inode, 0, buf, temp->fsize);

	terminal_write((char*) buf, temp->fsize);
//*/

/************************************ test rtc *************************************

	clear();
	int32_t* freq_ptr;
	int32_t k,x;
	char work[] = "1";
	char space[] = "\n";
	x = 2;
	freq_ptr = &x;
	while(x <= 1024)
	{
		rtc_write(freq_ptr);
		for (k = 0; k < 20; k++)
		{
		rtc_read();
		terminal_write(work, 1);
	    }
		terminal_write(space, 1);
		*freq_ptr *= 2;
	}

*/
/************************************ keyboard read ********************************
	char terminal_input[128];
	terminal_write(terminal_input, terminal_read(terminal_input, 128));
*/

	//* Execute the first program (shell') ...
	init_callbacks();
	root_shell();
	//uint8_t fname[] = "shell";
  //int result = prac_sys_call(2, fname);
	//printf("done! %d \n ", result);
	//*/

	/*
	 * Function: terminal_read
	 * Input: 	in_buf - buffer to be written to
	 *        	length - length of the input buffer
	 * Output:	returns the number of sucessfully read chars
	 *          -1 if an error occured
	 */
	/*
	char buf[128];
	printf("idt: %x \n \n", idt);
	open_file* cur_file = NULL;
	terminal_read(1, buf, 128, cur_file);
	printf(" \ndone");
	//*/
	/* Spin (nicely, so we don't chew up cycles) */
	asm volatile(".1: hlt; jmp .1;");
}
