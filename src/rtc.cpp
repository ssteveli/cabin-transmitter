#include "rtc.h"
#include "sensors/ds1307.h"
#include "config.h"

DS1307 ds1307(CB_I2C_SDA, CB_I2C_SCL);

static  const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; // API starts months from 1, this array starts from 0
 
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

time_t rtc_make_time(const rtc_time_t *tm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm->year * (SECS_PER_DAY * 365);

  for (i = 0; i < tm->year; i++) {
    if (LEAP_YEAR(i)) {
      seconds += SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm->month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm->year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }

  seconds+= (tm->date-1) * SECS_PER_DAY;
  seconds+= tm->hours * SECS_PER_HOUR;
  seconds+= tm->minute * SECS_PER_MIN;
  seconds+= tm->second;

  return (time_t)seconds; 
}

time_t rtc_read_time() {
  rtc_time_t t;
  if (rtc_read_time(&t) == 0) {
    return rtc_make_time(&t);
  }

  return 0;
}
