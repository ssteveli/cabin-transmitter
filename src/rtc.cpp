#include "rtc.h"
#include "sensors/ds1307.h"
#include "config.h"

DS1307 ds1307(CB_I2C_SDA, CB_I2C_SCL);

void rtc_start() {

}

int rtc_read_time(rtc_time_t *t) {
    if (t == NULL) return -1;

    uint8_t second;
    uint8_t minute;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;

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