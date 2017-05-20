// bOSs_coders
// ps2_keyboard.c
//
// Contains the initialization of the keyboard and the getchar() function
//
// Andrew Smith - 3/26/17
//
// ------------------------------------------------------------------------------------------------



// Dependencies:
// ------------------------------------------------------------------------------------------------
#include "ps2_keyboard.h"
#include "i8259.h"
#include "lib.h"


// Definitions:
// ------------------------------------------------------------------------------------------------
#define PS2_KB_PORT     0x60
#define PS2_KB_IRQ      0x01

#define SC_TABLE_SIZE   116
#define SHIFT_OFFSET    'A' - 'a'

#define CTRL_SC         0x01D
#define CTRL_SC_R       0x09D
#define DEL_SC          0x00E
#define L_SHIFT_SC      0x02A
#define L_SHIFT_SC_R    0x0AA
#define R_SHIFT_SC      0x036
#define R_SHIFT_SC_R    0x0B6
#define CL_SC           0x03A
#define L_ALT           0x038
#define R_ALT           0x0E0
#define L_ALT_R         0x0B8
#define R_ALT_R         0x0E0
#define F1              0x03B
#define F2              0x03C
#define F3              0x03D



static volatile unsigned int alt_on = OFF;
static volatile unsigned int ctrl_on = OFF;
static volatile unsigned int shift_on = OFF;
static volatile unsigned int caps_lock_on = OFF;
static int buf_end[TOTAL_TERMS] = {0, 0, 0};

// Scancode Table:
// ------------------------------------------------------------------------------------------------
char scancode[] = {
    0, 0,
    0, 0,
    '1', '!',
    '2', '@',
    '3', '#',
    '4', '$',
    '5', '%',
    '6', '^',
    '7', '&',
    '8', '*',
    '9', '(',
    '0', ')',
    '-', '_',
    '=', '+',
    0, 0,
    0, 0,
    'q', 'Q',
    'w', 'W',
    'e', 'E',
    'r', 'R',
    't', 'T',
    'y', 'Y',
    'u', 'U',
    'i', 'I',
    'o', 'O',
    'p', 'P',
    '[', '{',
    ']', '}',
    '\n', '\n',
    0, 0,
    'a', 'A',
    's', 'S',
    'd', 'D',
    'f', 'F',
    'g', 'G',
    'h', 'H',
    'j', 'J',
    'k', 'K',
    'l', 'L',
    ';', ':',
    '\'', '\"',
    '`', '~',
    0, 0,
    '\\', '|',
    'z', 'Z',
    'x', 'X',
    'c', 'C',
    'v', 'V',
    'b', 'B',
    'n', 'N',
    'm', 'M',
    ',', '<',
    '.', '>',
    '/', '?',
    0, 0,
    '*', 0,
    0, 0,
    ' ', ' ',
};


// Functions:
// ------------------------------------------------------------------------------------------------
/*
 * Function: initialize_kb
 * Input:    none
 * Output:   none
 *    Turns on the keyboard IRQ as well as initializes the keyboard buffer
 */
void initialize_kb(void){
    int i;

	keyboard_bufs[0] = keyboard_buf1;
	keyboard_bufs[1] = keyboard_buf2;
	keyboard_bufs[2] = keyboard_buf3;
    for(i = 0; i < KBD_BUF_SIZE; i++){
        keyboard_bufs[0][i] = -1; //Initialize terminal 1 keyboard
        keyboard_bufs[1][i] = -1; //Initialize terminal 2 keyboard
        keyboard_bufs[2][i] = -1; //Initialize terminal 3 keyboard
    }
	
    enable_irq(PS2_KB_IRQ);
}



/*
 * Function: getScancode
 * Input:    none
 * Output:   fetches the scancode from the keyboard
 *     returns the scancode and sets appropriate flags based on the key
 *     pressed
 */
static int getScancode(void) {
    int sc = (int)(unsigned char)inb(PS2_KB_PORT);
    int sc2 = 0;

    if(sc == R_ALT) {
        sc2 = (int)(unsigned char)inb(PS2_KB_PORT);
    }

    // Support for CTRL-L:
    if(sc == CTRL_SC) {
        ctrl_on = ON;
    }
    if(sc == CTRL_SC_R) {
        ctrl_on = OFF;
    }

    // Enable Caps Lock:
    if(sc == CL_SC && caps_lock_on == ON) {
        caps_lock_on = OFF;
    }
    else if(sc == CL_SC && caps_lock_on == OFF) {
        caps_lock_on = ON;
    }

    // Shift Letters:
    if(sc == L_SHIFT_SC || sc == R_SHIFT_SC) {
        shift_on = ON;
    }
    if(sc == L_SHIFT_SC_R || sc == R_SHIFT_SC_R) {
        shift_on = OFF;
    }

	// Alt:
    if(sc == L_ALT || sc2 == L_ALT) {
        alt_on = ON;
    }
    if(sc == L_ALT_R || sc2 == L_ALT_R){
        alt_on = OFF;
    }

    return sc;
}



/*
 * Function: getchar
 * Input:    none
 * Output:   the value of the char read from the keyboard
 *     gets the scancode from the keyboard and then converts it to the
 *     char it is supposed to represent.
 */
int getchar() {
    int c = getScancode();
    int key;

    if(c == F1 && alt_on == ON) {
        return ALTF1;
    }
     if(c == F2 && alt_on == ON) {
        return ALTF2;
    }
     if(c == F3 && alt_on == ON) {
        return ALTF3;
    }


    if(c < SC_TABLE_SIZE/2 && c >= 0) {
        key = scancode[2*c];

        if(c == DEL_SC) {
            return BKSP;
        }

        if(key == 'l' && ctrl_on == ON) {
            return CTRL_L;
        }

        if(shift_on == OFF && caps_lock_on == ON) {
            if(scancode[2*c] >= 'a' && scancode[2*c] <= 'z') {
	        key = scancode[2*c + 1];
	    }

        }
        else if(shift_on == ON && caps_lock_on == OFF) {
	    key = scancode[2*c + 1];
        }
        else if (shift_on == ON && caps_lock_on == ON) {
            if(scancode[2*c] >= 'a' && scancode[2*c] <= 'z') {
	        key = scancode[2*c];
	    }
	    else {
                key = scancode[2*c + 1];
	    }
	}
	else{
	    key = scancode[2*c];
        }

        return key;
    }

    return 0;
}

/*
 * Function: keyboard_handler
 * Input:    none
 * Output:   none
 *    Called by the interrupt handler to write a key to the keyboard buffer
 *    and terminal
 */
void keyboard_handler(void) {
    int c = getchar();
    if(c != 0) {
        add_kbd_buf((int8_t*)(keyboard_bufs[cur_term]), c, buf_end);
    }

    // Send EOI:
    send_eoi(PS2_KB_IRQ);
}

/*
 * Helper function to let halt clear the keyboard buffer when it runs
 */
void set_buf_end(int new_buf_end){
	buf_end[cur_term] = new_buf_end;
}
