#include "paging.h"
#include "pcb.h"
#include "file_system.h"
#define VIDEO 0xB8000
#define KB 1024
#define KB_4 4096
#define KERNEL_START 0x400000
#define MB 0x100000
#define BUFFER_SIZE 128

#define NO         0x0
#define YES        0x1
#define PRESENT    0x0001
#define PRIV       0x0004

#define VID_MEM_PAGE 1023

directory page_directory;
table page_table;
table video_page_table;

/*
 * paging_init
 *   DESCRIPTION: fills the paging arrays and initials paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Fills cr3 cr0 registers, allows virtual memory
 *   note: used wiki.osdev.org/ as reference
 */
void paging_init()
{
        long i = 0;

        //"first 4 MB of memory should be broken down into 4kB pages"
        for(i = 0; i < KB; ++i)
        {
                page_table.page[i] = KB_4*i;
        }

        // initialize the user video memory page:
	for(i = 0; i < KB; ++i)
        {
                video_page_table.page[i] = 0x000000000;
        }

        //video memory
        page_table.page[VIDEO/KB_4] = VIDEO;
        page_table.present = 1;
        page_table.read_write = 0; //1 for read/write
        page_table.user = 0; //if 1 anyone can access
        page_table.write_through = 0; //1 for write through caching
        page_table.cache_dis = 0; //1 if you don't want caching
        page_table.accessed =0; //1 if it has been read/written
        page_table.dirty = 0; //1 if it has been written to
        page_table.should_be_zero = 0;
        page_table.global = 1; //if set prevents tlb from updating the address if cr3 is reset
        set_page(&page_table, VIDEO/KB_4);

        //marks everything as not present
        for (i = 1; i < KB; ++i){
                page_directory.table[i] = 0x00000000;
        }

        //video memory
        page_directory.table[0] = (unsigned int) (page_table.page);
        page_directory.present = 1;  // 1 for present
        page_directory.read_write = 0; //1 for read/write
        page_directory.user = 1; //if 1 anyone can access
        page_directory.write_through = 0; //1 for write through caching
        page_directory.cache_dis = 0; //1 if you don't want caching
        page_directory.accessed = 0; //1 if it has been read/written
        page_directory.should_be_zero = 0;
        page_directory.size = 0; //1 for 4 mib pages
        page_directory.global = 1; //ignored
        set_table(&page_directory, 0);

        //virtual video memory
        page_directory.table[VID_MEM_PAGE] = (unsigned int) (video_page_table.page);
        page_directory.present = 1;  // 1 for present
        page_directory.read_write = 1; //1 for read/write
        page_directory.user = 1; //if 1 anyone can access
        page_directory.write_through = 0; //1 for write through caching
        page_directory.cache_dis = 0; //1 if you don't want caching
        page_directory.accessed = 0; //1 if it has been read/written
        page_directory.should_be_zero = 0;
        page_directory.size = 0; //1 for 4 mib pages
        page_directory.global = 1; //ignored
        set_table(&page_directory, VID_MEM_PAGE);

        //map kernel code
        page_directory.table[1] = KERNEL_START;
        page_directory.size = 1; //1 for 4 mib pages
        set_table(&page_directory, 1);

        //SET_PAGING (page_dir_addr);
        set_cr_reg(page_directory.table);
        return;
}


// next_free_vidmem
//     returns the index of the next free page dircetory entry:
unsigned int next_vidmap(void) {
    uint32_t i;
    for(i = 0; i < KB; i++) {
        if((video_page_table.page[i] & PRESENT) == NO) {
            break;
	    }
    }
    return i;
}


void set_table(directory * curr_dir, int i)
{
        int j = 0;
        if (curr_dir->present==0 || curr_dir->present == 1){
                curr_dir->table[i] = curr_dir->table[i] | curr_dir->present;
        }
        j += 1;

        if (curr_dir->read_write==0 || curr_dir->read_write == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->read_write << j);
        }
        j += 1;

        if (curr_dir->user==0 || curr_dir->user == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->user << j);
        }
        j += 1;

        if (curr_dir->write_through==0 || curr_dir->write_through == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->write_through << j);
        }
        j += 1;

        if (curr_dir->cache_dis==0 || curr_dir->cache_dis == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->cache_dis << j);
        }
        j += 1;

        if (curr_dir->accessed==0 || curr_dir->accessed == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->accessed << j);
        }
        j += 1;


        if (curr_dir->should_be_zero==0 || curr_dir->should_be_zero == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->should_be_zero << j);
        }
        j += 1;

        if (curr_dir->size==0 || curr_dir->size == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->size<< j);
        }
        j += 1;

        if (curr_dir->global==0 || curr_dir->global == 1){
                curr_dir->table[i] = curr_dir->table[i] | (curr_dir->global << j);
        }
        j = 0;
}

void set_page(table * curr_table, int i)
{
        int j = 0;
        if (curr_table->present==0 || curr_table->present == 1){
                curr_table->page[i] = curr_table->page[i] | curr_table->present;
        }
        j += 1;

        if (curr_table->read_write==0 || curr_table->read_write == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->read_write << j);
        }
        j += 1;

        if (curr_table->user==0 || curr_table->user == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->user << j);
        }
        j += 1;

        if (curr_table->write_through==0 || curr_table->write_through == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->write_through << j);
        }
        j += 1;

        if (curr_table->cache_dis==0 || curr_table->cache_dis == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->cache_dis << j);
        }
        j += 1;

        if (curr_table->accessed==0 || curr_table->accessed == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->accessed << j);
        }
        j += 1;

        if (curr_table->dirty==0 || curr_table->dirty == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->dirty << j);
        }
        j += 1;

        if (curr_table->should_be_zero==0 || curr_table->should_be_zero == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->should_be_zero << j);
        }
        j += 1;

        if (curr_table->global==0 || curr_table->global == 1){
                curr_table->page[i] = curr_table->page[i] | (curr_table->global << j);
        }

 }
