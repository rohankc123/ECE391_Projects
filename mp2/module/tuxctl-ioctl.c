/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

char last_led_state[6];
unsigned long buttons = 11111111;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	//printk("packet : %x %x %x\n", a, b, c);

	switch(a){
		case MTCP_RESET:
		{
			char buf[6];
			//Reset
			buf[0] = MTCP_RESET_DEV;
			tuxctl_ldisc_put(tty, &buf[0], 1);
			//Turn on Bioc
			buf[0] = MTCP_BIOC_ON;
			tuxctl_ldisc_put(tty, &buf[0], 1);
			//LED User Mode
			buf[0] = MTCP_LED_USR;
			tuxctl_ldisc_put(tty, &buf[0], 1);
			//Reset LEDs
			if(tuxctl_ldisc_put(tty, last_led_state, 6) != 0){
				printk("ERROR writing LED's\n");
			}
			return;
		}
		case MTCP_BIOC_EVENT:
		{
			buttons = 11111111;
			//Directions
			if((c&0x08) != 0) //Right
				buttons-=10000000;
			if((c&0x04) != 0) //Down
				buttons-=100000;
			if((c&0x02) != 0) //Left
				buttons-=1000000;
			if((c&0x01) != 0) //Up
				buttons-=10000;
			//Buttons
			if((b&0x08) != 0) //C
				buttons-=1000;
			if((b&0x04) != 0) //B
				buttons-=100;
			if((b&0x02) != 0) //A
				buttons-=10;
			if((b&0x01) != 0) //S
				buttons-=1;
			return;
		}
		case MTCP_ACK:
		
			return;
		default:
			return;
	}


}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int
tuxctl_ioctl (struct tty_struct* tty, struct file* file,
	      unsigned cmd, unsigned long arg)
{

    switch (cmd) {
	case TUX_INIT:
	{
		char buf[6];
		int i;
		buf[0] = MTCP_RESET_DEV;
		tuxctl_ldisc_put(tty, &buf[0], 1);

		buf[0] = MTCP_BIOC_ON;
		tuxctl_ldisc_put(tty, &buf[0], 1);

		buf[0] = MTCP_LED_USR;
		tuxctl_ldisc_put(tty, &buf[0], 1);
		buf[0] = MTCP_LED_SET;
		buf[1] = 0x0F;
		buf[2] = DISP_BLANK;
		buf[3] = DISP_BLANK;
		buf[4] = DISP_BLANK;
		buf[5] = DISP_BLANK;
		if(tuxctl_ldisc_put(tty, buf, 6) == 0){
			for(i = 0; i < 6; i++){
				last_led_state[i] = buf[i];
			}
		}
		else{
			printk("ERROR writing LED's\n");
		}
		return 0;
	}
	case TUX_BUTTONS:
	{
		if(arg == 0x0){ //Check if NULL
			return -EINVAL;
		}
		else{
			unsigned long* button_mem_location;
			button_mem_location = (unsigned long*) arg;
			*button_mem_location = buttons;
			return 0;
		}
	}
	case TUX_SET_LED:
	{
		char buf[6];
		//get LED's from low 4 from byte 3
		int bytes_one_two = (arg)&(0xFFFF);
		int byte_three = (arg>>16)&0xF;
		int byte_four = (arg>>24)&0xF;

		int led_zero_en = (byte_three & 0x01);
		int led_one_en = ((byte_three>>1) & 0x01);
		int led_two_en = ((byte_three>>2) & 0x01);
		int led_three_en = ((byte_three>>3) & 0x01);

		int i;
		int j = 2;

		buf[0]=MTCP_LED_SET;
		buf[1]=0x0F;


		//get value to display from first two bytes
		for(i=3; 3-i<4; i--){
			if(led_zero_en == 0 && (3-i)==0){
				if((byte_four&0x01)==0)
					buf[j] = DISP_BLANK;
				else
					buf[j] = DISP_BLANK_D;
				j++;
				continue;
			}
			if(led_one_en == 0 && (3-i)==1){
				if((byte_four&0x02)==0)
					buf[j] = DISP_BLANK;
				else
					buf[j] = DISP_BLANK_D;
				j++;
				continue;
			}
			if(led_two_en == 0 && (3-i)==2){
				if((byte_four&0x04)==0)
					buf[j] = DISP_BLANK;
				else
					buf[j] = DISP_BLANK_D;
				j++;
				continue;
			}
			if(led_three_en == 0 && (3-i)==3){
				if((byte_four&0x08)==0)
					buf[j] = DISP_BLANK;
				else
					buf[j] = DISP_BLANK_D;
				j++;
				continue;
			}
			if((byte_four>>(3-i)&0x1)){
				switch((bytes_one_two>>((3-i)*4))&0x000F){
					case 0x0:
						buf[j]=DISP_ZERO_D;
						break;
					case 0x1:
						buf[j]=DISP_ONE_D;
						break;
					case 0x2:
						buf[j]=DISP_TWO_D;
						break;
					case 0x3:
						buf[j]=DISP_THREE_D;
						break;
					case 0x4:
						buf[j]=DISP_FOUR_D;
						break;
					case 0x5:
						buf[j]=DISP_FIVE_D;
						break;
					case 0x6:
						buf[j]=DISP_SIX_D;
						break;
					case 0x7:
						buf[j]=DISP_SEVEN_D;
						break;
					case 0x8:
						buf[j]=DISP_EIGHT_D;
						break;
					case 0x9:
						buf[j]=DISP_NINE_D;
						break;
					case 0xA:
						buf[j]=DISP_A_D;
						break;
					case 0xB:
						buf[j]=DISP_B_D;
						break;
					case 0xC:
						buf[j]=DISP_C_D;
						break;
					case 0xD:
						buf[j]=DISP_D_D;
						break;
					case 0xE:
						buf[j]=DISP_E_D;
						break;
					case 0xF:
						buf[j]=DISP_F_D;
						break;
				}
			}
			else{
				switch((bytes_one_two>>((3-i)*4))&0x000F){
					case 0x0:
						buf[j]=DISP_ZERO;
						break;
					case 0x1:
						buf[j]=DISP_ONE;
						break;
					case 0x2:
						buf[j]=DISP_TWO;
						break;
					case 0x3:
						buf[j]=DISP_THREE;
						break;
					case 0x4:
						buf[j]=DISP_FOUR;
						break;
					case 0x5:
						buf[j]=DISP_FIVE;
						break;
					case 0x6:
						buf[j]=DISP_SIX;
						break;
					case 0x7:
						buf[j]=DISP_SEVEN;
						break;
					case 0x8:
						buf[j]=DISP_EIGHT;
						break;
					case 0x9:
						buf[j]=DISP_NINE;
						break;
					case 0xA:
						buf[j]=DISP_A;
						break;
					case 0xB:
						buf[j]=DISP_B;
						break;
					case 0xC:
						buf[j]=DISP_C;
						break;
					case 0xD:
						buf[j]=DISP_D;
						break;
					case 0xE:
						buf[j]=DISP_E;
						break;
					case 0xF:
						buf[j]=DISP_F;
						break;
				}
			}
			j++;
		}
		if(tuxctl_ldisc_put(tty, buf, j) == 0)
			for(i = 0; i < 6; i++)
				last_led_state[i] = buf[i];
		else
			printk("ERROR writing LED's\n");

		return 0;
	}
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
    return 0;
}

