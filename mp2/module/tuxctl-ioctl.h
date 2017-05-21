// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)

#define DISP_ZERO		0xE7
#define DISP_ZERO_D		0xF7
#define DISP_ONE		0x6
#define DISP_ONE_D		0x16
#define DISP_TWO		0xCB
#define DISP_TWO_D		0xDB
#define DISP_THREE		0x8F
#define DISP_THREE_D	0x9F
#define DISP_FOUR		0x2E
#define DISP_FOUR_D		0x3E
#define DISP_FIVE		0xAD
#define DISP_FIVE_D		0xBD
#define DISP_SIX		0xED
#define DISP_SIX_D		0xFD
#define DISP_SEVEN		0x86
#define DISP_SEVEN_D	0x96
#define DISP_EIGHT		0xEF
#define DISP_EIGHT_D	0xFF
#define DISP_NINE		0xAF
#define DISP_NINE_D		0xBF
#define DISP_A			0xEE
#define DISP_A_D		0xFE
#define DISP_B			0x6D
#define DISP_B_D		0x7D
#define DISP_C			0xE1
#define DISP_C_D		0xF1
#define DISP_D			0x4F
#define DISP_D_D		0x5F
#define DISP_E			0xE9
#define DISP_E_D		0xF9
#define DISP_F			0xE8
#define DISP_F_D		0xF8
#define DISP_BLANK 		0x00
#define DISP_BLANK_D	0x10


#endif

