#ifndef PAGING_H
#define PAGING_H

#define KB 1024
#define KB_4 4096
#define KERNAL_START 0x400000
#define VIDEO_1   0x2000000
#define VIDEO_2   0x2001000
#define VIDEO_3   0x2002000
#define VID_MEM_PAGE 1023

typedef struct _directory
{
    unsigned int table[KB] __attribute__((aligned(KB_4)));
    int present;  // 1 for present
    int read_write; //1 for read/write
    int user; //if 1 anyone can access
    int write_through; //1 for write through caching
    int cache_dis; //1 if you don't want caching
    int accessed; //1 if it has been read/written
    int should_be_zero;
    int size; //1 for 4 mib pages
    int global; //ignored

} directory;

typedef struct _table
{
    unsigned int page[KB] __attribute__((aligned(KB_4)));
    int present;
    int read_write; //1 for read/write
    int user; //if 1 anyone can access
    int write_through; //1 for write through caching
    int cache_dis; //1 if you don't want caching
    int accessed; //1 if it has been read/written
    int dirty; //1 if it has been written to
    int should_be_zero;
    int global; //if set prevents tlb from updating the address if cr3 is reset
} table;

extern void set_table(directory * curr_dir, int i);
extern void set_page(table * curr_table, int i);
extern void set_cr_reg(void *);
extern void paging_init();
extern unsigned int next_vidmap(void);
extern directory page_directory;
extern table page_table;
extern table video_page_table;
extern table off_display_table;

#endif
