#include "lib.h"
#include "file_system.h"

#define FILE_SYS_START 0x00412000
#define ENTRY_SIZE 64
#define FSYSTEM_SIZE 63
#define BLOCK_SIZE 4096
#define TYPE0 3
#define TYPE1 5
#define TYPE2 9

#define INODE_SHIFT_3 24
#define INODE_SHIFT_2 16
#define INODE_SHIFT_1 8

#define BYTE_3 3
#define BYTE_2 2
#define BYTE_1 1

#define NEXT_ADDR 4
uint32_t file_system_addr;

/*
 * Function: read_dentry_by_name
 * Input:    fname:	name of file
 * 	     dentry: 	dentry_t block
 * Return:   0  if successful
 *	     -1 if non existent file
 *
 * When successful, fills dentry_t block with file name (32 bytes), file type (4 bytes), inode# (4 bytes) for the file
 */
int32_t read_dentry_by_name(uint8_t* fname, dentry_t* dentry){
	int i;
	int found;
	int index = 0;
	int byte_off;
	uint8_t* curr_address;

	//iterate through every index
	while(index < FSYSTEM_SIZE){
		found = 0;

		//offset to specific index
		byte_off = ((index + 1) * ENTRY_SIZE);
		//address of first directory entry
	 	curr_address = (uint8_t*)(byte_off + file_system_addr);
		//End of file system
		if(curr_address[0] == '\0')
			break;

	 	for(i = 0; i < NAME_LENGTH; i++){
	 		//if name doesn't match, check next index
		 	if (curr_address[i] == '\0') {
				if(fname[i] == '\0'){
					found = 1;
				}
				else{
					found = 0;
				}
				break;
			}
		 	if (fname[i] == '\0' || curr_address[i] != fname[i]){
				found = 0;
		 		break;
		 	}
		}
		if (i == NAME_LENGTH) {
			if(fname[i] == '\0') found = 1;
		}
	 	//if word matches, fill dentry
	 	if (found == 1){
			//iterate through every character
			//uint8_t fname[32]
			for(i = 0; i < NAME_LENGTH; i++){
				dentry->fname[i] = *curr_address;
				curr_address++;
			}

			//uint8_t ftype
			dentry->ftype = *curr_address;

			//uint32_t inode
			curr_address += NEXT_ADDR;


			dentry->inode = (*(curr_address + BYTE_3) << INODE_SHIFT_3) + (*(curr_address + BYTE_2) << INODE_SHIFT_2) + (*(curr_address + BYTE_1) << INODE_SHIFT_1) + (*curr_address);

			//uint32_t size
			dentry->fsize = *((uint32_t*)(file_system_addr + (1 + dentry->inode) * BLOCK_SIZE));

			return 0;
 		}

 		//else, check next index
		index += 1;
	}

	//non existent file
	return -1;
}

/*
 * Function: read_dentry_by_index
 * Input:    index:	index number
 * 	     dentry: 	dentry_t block
 * Return:   0  if successful
 *	     -1 if invalid index
 *
 * When successful, fills dentry_t block with file name (32 bytes), file type (4 bytes), inode number (4 bytes) for the file
 */

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
	int i;
	int byte_off;
	uint8_t* curr_address;

	//invalid index
	if((index > FSYSTEM_SIZE - 1) || (index < 0)){
		return -1;
	}

	//offset to specific index
	byte_off = ((index + 1) * ENTRY_SIZE);
	//address of first directory entry
 	curr_address = (uint8_t*)(byte_off + file_system_addr);

	//iterate through every character
	//uint8_t fname[32]
	for(i = 0; i < NAME_LENGTH; i++){
		dentry->fname[i] = *curr_address;
		curr_address++;
	}

	//uint8_t ftype
	dentry->ftype = *curr_address;

	//uint32_t inode
	curr_address += NEXT_ADDR;



	dentry->inode = (*(curr_address + BYTE_3) << INODE_SHIFT_3) + (*(curr_address + BYTE_2) << INODE_SHIFT_2) + (*(curr_address + BYTE_1) << INODE_SHIFT_1) + (*curr_address);

	//uint32_t size
	dentry->fsize = *((uint32_t*)(file_system_addr + (1 + dentry->inode) * BLOCK_SIZE));

	return 0;
}

/*
 * Function: read_data
 * Input:    inode:	inode number
 * 	     offset:	position in file
 *	     buf: 	buffer to place read bytes
 *	     length: 	bytes to read
 * Return:   0  if end of file reached (if start is past end of file or length of file <= 0)
 *	     -1 if invalid inode number or if bad data block number is found within file bounds of given inode
 * 	     else number of bytes read and placed in buffer
 *
 * read_data call can only check that the given inode is within the valid range
 * read_data call does not check that the inode actually corresponds to file (not all inodes are used)
 *
 * Works like read system call, reads up to 'length' bytes starting from 'offset' in file with 'inode'
 */
 //bytes_read = read_data(cur_file->inode, cur_file->filepos, buf, n_bytes);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	int counter = 0;
	int first = 0;
	uint8_t* buf_start = buf;

	//starting block number
	uint32_t block_number = offset / BLOCK_SIZE;
	//offset within block
	uint32_t current_byte = offset % BLOCK_SIZE;

	//addr of beginning of inode
	uint32_t* inode_ptr = (uint32_t*)(file_system_addr + ((1 + inode) * BLOCK_SIZE));
	//addr of data block
	uint32_t* block_ptr = inode_ptr + block_number + 1;
	//data block number
	uint32_t data_block_number = (uint32_t)*block_ptr;
	//pointing to character
	uint8_t* char_ptr = (uint8_t*)(file_system_addr + (BLOCK_SIZE * (ENTRY_SIZE + 1 + data_block_number)));
	//pointer relative to offset
	char_ptr += current_byte;
	//length of file
	uint32_t length_of_file = *inode_ptr;

	//if start is past end of file or length of file <= 0
	if((offset > length_of_file) || (length_of_file <= 0)){
		return 0;
	}

	//if invalid inode number, return -1
	if((inode > (FSYSTEM_SIZE)) || (inode < 0)){
		return -1;
	}

	//if starting at top of block, set edge case
	if(current_byte == 0){
		first = 1;
	}

	//loop until read up to length
	while(length > 0){
		//check location of byte relative to block
		current_byte = current_byte % BLOCK_SIZE;
		//if at top of block
		if(current_byte == 0){
			//if not first start
			if(first == 0){
				block_ptr++;
				//data block number
				data_block_number = (uint32_t)*block_ptr;
				//pointing to character
				char_ptr = (uint8_t*)(file_system_addr + (BLOCK_SIZE * (ENTRY_SIZE + 1 + data_block_number)));
			}
			else{
				first = 0;
			}
		}

		//if bad data block number is found within file bounds of given inode, return -1
		if(!block_ptr){
			return -1;
		}

		//fill buffer with bytes read
		*buf = *char_ptr;

		//stop reading if reached end of file
		offset++;
		if(offset > length_of_file){

			break;
		}
		length--;
		current_byte++;
		buf++;
		char_ptr++;
		counter++;
	}
	buf = buf_start;
	return counter;
}

int32_t file_open(const uint8_t* filename){
	return 0;
}

int32_t file_read(int32_t fd, void* _buf, int32_t n_bytes, void* _cur_file){
	uint8_t* buf = (uint8_t*)_buf;
	open_file* cur_file = (open_file*)_cur_file;

	int32_t bytes_read = 0;
	if (cur_file->flags == 0 || !buf ){ //
		return -1;
	}

	bytes_read = read_data(cur_file->inode, cur_file->filepos, buf, n_bytes);
	cur_file->filepos += bytes_read;
	return bytes_read;
}

int32_t file_write(int32_t fd, const void* _buf, int32_t n_bytes, void* _cur_file){
	(void)_buf;
	(void)fd;
	(void)n_bytes;
	(void)_cur_file;
	return -1; // *phew! that was hard*
}

int32_t file_close(void) {
	return 0;
}


int32_t dir_open(const uint8_t* filename) {
	return 0;
}

int32_t dir_read(int32_t fd, void* _buf, int32_t n_bytes, void* _cur_file){
	int return_value;
	uint8_t* buf = (uint8_t*)_buf;
	open_file* cur_file = (open_file*)_cur_file;
	int i;
	if (cur_file->flags == 0 || !buf){
		return -1;
	}
	dentry_t new_entry;
	dentry_t* new_entry_ptr = &new_entry;
	return_value = read_dentry_by_index(cur_file->filepos, new_entry_ptr);
	if (return_value == -1){
		return 0;
	}
	cur_file->filepos++;
	for(i = 0; i < NAME_LENGTH; i++){
		if (new_entry_ptr->fname[i] == 0) break;
	}

	//fname size
	memcpy(buf, new_entry_ptr->fname, 32); //length of name

	return i;
}

int32_t dir_write(int32_t fd, const void* _buf, int32_t n_bytes, void* _cur_file){
    (void)fd;
    (void)_buf;
    (void)_cur_file;
    (void)n_bytes;
    return -1; // only thing harder was file_read
}

int32_t dir_close(void) {
	return 0;
}
