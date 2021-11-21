#include "rtc.h"
#include "sensors/ds1307.h"
#include "mbed.h"

DS1307 ds1307(D14, D15);

void rtc_start() {

}

int rtc_read(rtc_time_t *t) {
    if (t == NULL) return -1;

    int second;
    int minute;
    int hours;
    int day;
    int date;
    int month;
    int year;

    int result = ds1307.gettime(
        &second,
        &minute,
        &hours,
        &day,
        &date,
        &month,
        &year
    );

    if (result == 0) {
        t->second = second;
        t->minute = minute;
        t->hours = hours;
        t->day = day;
        t->date = date;
        t->month = month;
        t->year = year;
    }

    return result;
}