
#ifndef PCB_H
#define PCB_H

#include "ps2_keyboard.h"
#include "system_call.h"

#define FD_NUM 8

// struct __device
//     this struct holds the function pointers for a devices
//     read write open close functions
typedef struct __device {
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes, void* file);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes, void* file);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(void);
} device;

typedef struct _open_file {
    struct __device table_pointer;
    uint32_t inode;
    uint32_t filepos;
    uint32_t flags;
    uint32_t pid; //which task are we in?
    //might need to add child or w/e
} open_file;

typedef struct _PCB
{
  uint32_t esp_reg;
  uint32_t ebp_reg;
  struct _PCB* parent_pcb;
  struct _PCB* child_pcb;
  uint32_t pid;
  open_file open_files[FD_NUM];
  uint8_t args_buf[KBD_BUF_SIZE-1];
  uint32_t vidmem_idx;                 // index of vidmem so when halted it can be freed
} PCB;


extern PCB* get_pcb();

#endif

