// _bOSs_coders
// RTC - Initialization and driver
// Andrew Smith
//     3/11/17
//-------------------------------------------------------------------

#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "pcb.h"

// RTC Device Definitions
//-------------------------------------------------------------------
#define RTC_PORT        0x70
#define RTC_CMOS        0x71
#define RTC_REG         0x70
#define RTC_STATUS_A    0x0A
#define RTC_STATUS_B    0x0B
#define RTC_STATUS_C    0x0C
#define RTC_IRQ         0x8
#define RTC_STATUS      0x80
#define NUM_PROC 3

// RTC Functions
//-------------------------------------------------------------------
// Handles the rtc interrupts
int volatile rtc_flag[NUM_PROC];

extern void rtc_handler(void);

// rtc_init: initializes the rtc chip
extern void rtc_init();

// rtc_enable: turns on the periodic interrupt
extern void rtc_enable();

// rtc_diable: turns off the periodic interrupt
extern void rtc_disable();

// rtc_set_rate: sets a new frequency for the RTC chip
extern void rtc_set_rate(unsigned char rate);

// rtc_eoi: reads register c to allow next interrupt
extern void rtc_eoi();

//rtc driver functions
extern int32_t rtc_read(int32_t fd, void* buf, int32_t n_bytes, void* cur_file);

extern int32_t rtc_write(int32_t fd, const void* buf, int32_t n_bytes, void* cur_file);

extern int32_t rtc_open(const uint8_t* filename);

extern int32_t rtc_close(void);


#endif /* RTC_H */
