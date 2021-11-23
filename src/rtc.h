#ifndef SRC_RTC_
#define SRC_RTC_

#include "mbed.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

void rtc_start();
int rtc_read_time(rtc_time_t *t);

#endif