#ifndef SRC_RTC_
#define SRC_RTC_

typedef struct {
    int second;
    int minute;
    int hours;
    int day;
    int date;
    int month;
    int year;
} rtc_time_t;

void rtc_start();
int rtc_read(rtc_time_t **t);

#endif