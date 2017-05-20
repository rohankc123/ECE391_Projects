#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#define NAME_LENGTH 32
#include "pcb.h"
extern uint32_t file_system_addr;

typedef struct _dentry_t{
	uint8_t fname[NAME_LENGTH];
	uint8_t ftype;
	uint32_t inode;
	uint32_t fsize;
} dentry_t;

extern int32_t read_dentry_by_name(uint8_t* fname, dentry_t* dentry);

extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t file_open(const uint8_t* filename);

extern int32_t file_read(int32_t fd, void* _buf, int32_t n_bytes, void* _cur_file);

extern int32_t file_write(int32_t fd, const void* _buf, int32_t n_bytes, void* _cur_file);

extern int32_t file_close(void);

extern int32_t dir_open(const uint8_t* filename);

extern int32_t dir_read(int32_t fd, void* _buf, int32_t n_bytes, void* _cur_file);

extern int32_t dir_write(int32_t fd, const void* _buf, int32_t n_bytes, void* _cur_file);

extern int32_t dir_close(void);

extern int32_t entrypoint(uint32_t entry);
#endif
