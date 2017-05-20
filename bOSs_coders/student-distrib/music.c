#include "pit_init.h"
#include "pit.h"
#include "i8259.h"
#include "lib.h"
#include "rtc.h"

#define D4 293
#define G4 391
#define D5 587
#define C5 523
#define B4 493
#define A4 440
#define G5 783
#define F4s 369

#define _RTC_FREQ 8

void triplet(uint32_t first, uint32_t second, uint32_t third) {
    int j;
    play_sound(first);
    rtc_read(j, &j, j, &j);
    nosound();
    rtc_read(j, &j, j, &j);

    play_sound(second);
    rtc_read(j, &j, j, &j);
    nosound();
    rtc_read(j, &j, j, &j);

    play_sound(third);
    rtc_read(j, &j, j, &j);
    nosound();
    rtc_read(j, &j, j, &j);
}

void quarter(uint32_t note) {
    int i,j;
    play_sound(note);
    for(i = 0 ; i < 3; i++)
        rtc_read(j, &j, j, &j);

    nosound();

    for(i = 0; i < 2; i++)
        rtc_read(j, &j, j, &j);
}

void half(uint32_t note) {
    int i,j;
    play_sound(note);
    for(i = 0 ; i < 8; i++)
        rtc_read(j, &j, j, &j);

    nosound();

    for(i = 0; i < 2; i++)
        rtc_read(j, &j, j, &j);
}

void star_wars() {
    int i,j;
    int rtc_freq = _RTC_FREQ;
    rtc_write(j, &rtc_freq, 4, &j);
    for(i = 0; i < 2; i++) {
        triplet(D4,D4,D4);
        half(G4);
        half(D5);
        triplet(C5,B4,A4);
        half(G5);
        quarter(D5);
        triplet(C5,B4,A4);
        half(G5);
        quarter(D5);
        triplet(C5,B4,C5);
        half(A4);
    }
    rtc_freq = 2;
    rtc_write(j, &rtc_freq, 4, &j);
}
