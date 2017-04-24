#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

#define PCB_BITMASK 0xFFFFE000

extern void init_callbacks(void);

extern void system_call(int sys_call_number);

extern int32_t halt (uint8_t status);

extern int32_t execute (const uint8_t* command);

extern int32_t read (int32_t fd, void* buf, int32_t nbytes);

extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t open (const uint8_t* filename);

extern int32_t close (int32_t fd);

extern int32_t getargs (uint8_t* buf, int32_t nbytes);

extern int32_t vidmap (uint8_t** screen_start);

extern int32_t set_handler (int32_t signum, void* handler_address);

extern int32_t sigreturn (void);

extern int32_t root_shell(void);

extern void sys_halt(int32_t status);

#endif
