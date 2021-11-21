#ifndef DS1307_H
#define DS1307_H
 
/* mbed Dallas Semiconductor DS1307 serial real time clock
* Copyright (c) 2012 pksmith
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
 
#include "mbed.h"
 
#define DS1307_addr 0xD0    // this is fixed by Dallas
#define DS1307_freq 100000  // this is the Dallas spec for operating i2c for this device
#define DS1307_sec 0x00     // seconds
#define DS1307_min  0x01    // min
#define DS1307_hour 0x02    // hours
#define DS1307_day  0x03    // day
#define DS1307_date 0x04    // date
#define DS1307_month 0x05   // month
#define DS1307_year 0x06    // year
#define DS1307_sqrout 0x07  // square output register
#define DS1307_ramstart 0x08    // register address that ram starts at
#define DS1307_lastreg 0x3F // this is the last register in the device (note also this register is used to address everything so it gets clobbered)
#define DS1307_lastram 0x3E // last usable ram by this class as the lastreg is clobbered by code for normal operation
 
/** DS1307 control and communication class using mbed's i2c class
 *
 * Example:
 * @code
 * // show how the DS1307 class works
 * #include "ds1307.h"
 * #include "mbed.h"
 *
 * Serial pc(USBTX, USBRX); // tx, rx  for debug and usb pc comunications
 *
 * DS1307 my1307(p9,p10); // start DS1307 class and give it pins for connections of the DS1307 device
 *
 * int sec = 0;
 * int min = 0;
 * int hours = 0;
 * int day = 0;
 * int date = 0;
 * int month = 0;
 * int year = 0;
 *
 * void test_rw(int test) {
 *     if (test == 0) pc.printf("Last R/W operaion passed!\n\r");
 *     else pc.printf("Last R/W operation failed!\n\r");
 * }
 *
 * int main() {
 *     int junk = 0;
 *
 *     sec = 24;       // 24 seconds
 *     min = 13;       // 13 min
 *     hours = 13;     // 1 pm
 *     day = 4;        // wednesday
 *     date = 20;      // June 20
 *     month = 6;
 *     year = 12;      // 2012
 *                     // set time to these values on the ds1307 connected device
 *
 *     test_rw(my1307.settime( sec, min, hours, day, date, month, year));
 *     pc.printf("seconds set are %.2D \n\r",sec);
 *     pc.printf("min set are %.2D \n\r",min);
 *     pc.printf("hour set are %.2D \n\r",hours);
 *     pc.printf("day set are %.2D \n\r",day);
 *     pc.printf("date set are %.2D \n\r",date);
 *     pc.printf("month set are %.2D \n\r",month);
 *     pc.printf("year set are %.2D \n\r",year);
 *     wait(3);
 *                     // now read the time of the DS1307 device and see what time it is
 *                     // note that because of the 3 second wait this time should be 3 seconds past what it was set to earlier
 *
 *     test_rw(my1307.gettime( &sec, &min, &hours, &day, &date, &month, &year));
 *     pc.printf("seconds read are %.2D \n\r",sec);
 *     pc.printf("min read are %.2D \n\r",min);
 *     pc.printf("hour read are %.2D \n\r",hours);
 *     pc.printf("day read are %.2D \n\r",day);
 *     pc.printf("date read are %.2D \n\r",date);
 *     pc.printf("month read are %.2D \n\r",month);
 *     pc.printf("year read are %.2D \n\r",year);
 *
 *     junk = 0x39;                            // just a junk value do read and write test to DS1307 ram
 *     test_rw(my1307.write( 0x20, junk));     // this should write the value of junk to register 0x20 (a ram location) in the ds1307.
 *     pc.printf("Value written to register 0x20 %.2X \n\r",junk);
 *     junk = 0;                               // clear junk to show that when the register is read from the correct value is obtained
 *     test_rw(my1307.read( 0x20, &junk));     // this should read register 0x20
 *     pc.printf("Value read from register 0x20 %.2X \n\r",junk);
 * }
 * @endcode
 */
class DS1307 {
public:
    /** Create object connected to DS1307 pins ( remember both pins need pull up resisters)
    *
    * Ensure the pull up resistors are used on these pins.  Also note there is no checking on 
    *  if you use thes pins p9, p10, p27, p28 so ensure you only use these ones on the LPC1768 device
    *
    * @param sda pin that DS1307 connected to (p9 or p28 as defined on LPC1768)
    * @param slc pin that DS1307 connected to (p10 or p27 ad defined on LPC1768)
    */
    DS1307( PinName sda, PinName slc) ;                 // constructor
 
    ~DS1307();                                          // destructor
 
    /** Bulk read of several registers at a time
    *
    * Ensure the variable data pointer passed to this function has the room needed to recieve the quantity!
    *
    * @param addr the address to read from
    * @param quantity the amount of registers to read from
    * @param data the place to put the values read
    * @param returns 0 if read worked 1 if the read of DS1307 failed for some reason
    */
    int read( int addr, int quantity, char *data);      // to read some of the 63 bytes from DS1307
 
    /** Read one register of DS1307 device
    *
    * @param addr the address to read from
    * @param data read from the one register
    * @param returns 0 if read worked 1 if the read of DS1307 failed for some reason
    */
    int read(int addr, int *data);                      // to read one byte only
 
    /** Bulk write of several registers at a time
    *
    * @param addr the address to write to
    * @param quantity the amount of registers to write to
    * @param data that contains the values to be written to the registers
    * @param returns 0 if write worked 1 if the write to DS1307 failed for some reason
    */
    int write( int addr, int quantity, char *data);     // to write bytes to some of the 63 locations in the DS1307
 
    /** Write one register of DS1307 device
    *
    * @param addr the address to write to
    * @param data to write to register
    * @param returns 0 if write worked 1 if the write to DS1307 failed for some reason
    */
    int write( int addr, int data );                    // to write one byte only
 
    /** Start DS1307 clock
    *
    * @param returns 0 if clock started 1 if the write command to DS1307 failed for some reason
    */
    int start_clock(void);                              // start the clock
 
    /** Stop DS1307 clock
    *
    * @param returns 0 if clock stopped 1 if the write command to DS1307 failed for some reason
    */
    int stop_clock(void);                               // stop clock
 
    /** Set twelve hour mode on DS1307 (note this also converts 24 hour time to 12 time if needed on DS1307)
    *
    * Note this will convert DS1307 time values in registers to 12 hour values from 24 hour values if needed
    *
    * @param returns 0 if DS1307 is now in 12 hour mode 1 if the command to DS1307 failed for some reason
    */
    int twelve_hour(void);                              // set 12 hour mode
 
    /** Set twenty four hour mode on DS1307 
    *
    * Note this will convert DS1307 time values in registers to 24 hour values from 12 hour values if needed
    *
    * @param returns 0 if DS1307 is now in 24 hour mode 1 if the command to DS1307 failed for some reason
    */
    int twentyfour_hour(void);                          // set 24 hour mode
 
    /** Set the time to some current or other value ( note that this will start the clock after it is set!)
    *
    * Note this will return 1 if any of the values passed to this function are not as listed below!
    *
    * @param sec the seconds value (0 - 59)
    * @param min the minute value (0 - 59)
    * @param hour the hour value (0 - 23) always in 24 hour
    * @param day the day value ( sunday is 1 )
    * @param date the date value (1 - 31)
    * @param month the month value (1-12)
    * @param year the year value (00 - 99) this is for 2000 to 2099 only as i understand it!
    * @param returns 0 if time is set 1 if the time setting failed in some way
    */
    int settime(int sec, int min, int hour, int day, int date, int month, int year);         // to set the current time and start clock
 
    /** Read the current time of the DS1307
    *
    * @param sec the seconds value (0 - 59)
    * @param min the minute value (0 - 59)
    * @param hour the hour value (0 - 23) always in 24 hour
    * @param day the day value ( sunday is 1 )
    * @param date the date value (1 - 31)
    * @param month the month value (1-12)
    * @param year the year value (00 - 99) this is for 2000 to 2099 only as i understand it!
    * @param returns 0 if time is read correctly 1 if the time was not recieved correctly for some reason
    */
    int gettime(int *sec, int *min, int *hour, int *day, int *date, int *month, int *year);  // to get the current time information
 
 
protected:
    I2C ds1307i2c;
    int dectobcd( int );
    int bcdtodec( int );
    int hilow_check( int, int, int);
 
};
 
#endif