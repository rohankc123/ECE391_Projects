// bOSs_coders
// ps2_keyboard.h
//
// Contains the initialization of the keyboard and the getchar() function
//
// Andrew Smith - 3/12/17
//
// ------------------------------------------------------------------------------------------------

#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "types.h"

#define CTRL_L     		-10
#define BKSP     		-15

#define KBD_BUF_SIZE     129
volatile unsigned char keyboard_buf[KBD_BUF_SIZE];

/* reads from the keyboard buffer */
extern int keyboard_read(char* in_buf, unsigned int length);

/* keyboard interrupt handeler */
extern void keyboard_handler(void);

/* initialize the keyboard and enables its IRQ */
extern void initialize_kb(void);

/* Gets the last char entered from the keyboard */
extern int getchar(void);

/* Helper function for halt to clear terminal buffer */
extern void set_buf_end(int new_buf_end);

#endif
