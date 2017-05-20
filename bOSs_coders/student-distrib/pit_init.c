#include "pit_init.h"
#include "pit.h"
#include "i8259.h"
#include "lib.h"
#include "rtc.h"

/*
 * pit_init
 *   DESCRIPTION: initializes the pit
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: wrapper for rtc_enable, Enable (unmask) the IRQ 0
 */
 void pit_init() {
    pit_enable();
    enable_irq(0);
    return;
}

void play_sound  (uint32_t nFrequence)  {

 	uint32_t Div;
 	uint8_t tmp;

        //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0xb6, 0x43);
 	outb((uint8_t) (Div),0x42);
 	outb( (uint8_t) (Div >> 8) ,0x42);

        //And play the sound using the PC speaker
 	tmp = inb(0x61);
  	if (tmp != (tmp | 3)) {
 		outb(tmp | 3 ,0x61);
 	}
  return;

 }

 //make it shutup
 void nosound() {
 uint8_t tmp = inb(0x61) & 0xFC;

 outb(tmp,0x61);
}

//Make a beep
//int32_t fd, void* buf, int32_t n_bytes, void* cur_file
void beep() {
  int32_t i;
  play_sound(1000);
  rtc_read((int32_t)1, &i, i, &i);

         //set_PIT_2(old_frequency);
}

void boot_noyes() {
    int32_t freq = 50;
    int32_t i,j;
    int32_t rtc_freq = 8;
    rtc_write(j, &rtc_freq, 4, &j);
    for(i = 0; i < 20; i++) {
        play_sound(freq);
        freq += 100;
        rtc_read(j, &j, j, &j);
    }
    rtc_freq = 2;
    rtc_write(j, &rtc_freq, 4, &j);
    nosound();
}
