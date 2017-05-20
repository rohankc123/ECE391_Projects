#ifndef PIT_INIT_H
#define PIT_INIT_H

#include "types.h"

extern void pit_init();
extern void play_sound(uint32_t nFrequence);
extern void nosound();
extern void beep();
extern void boot_noyes();
#endif
