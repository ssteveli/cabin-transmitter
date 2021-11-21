#include "ds1307.h"
 
DS1307::DS1307(PinName sda, PinName scl ) : ds1307i2c(sda,scl) {
    ds1307i2c.frequency(DS1307_freq);
}
 
DS1307::~DS1307() {
}
 
int DS1307::read( int addr, int quantity, char *data) {
    // note the char array at data must contain 63 locations or unpredictable behavior will happen
    // addr must be 0 - 62 as the 64th(or 63rd as indexed from 0) ram location is clobered in this method of access
    // quantity must be 1 - 63 as the 64th ram location is clobered in this method of access
    int test = 0 ;
    char temp_data[65];
 
    if (addr > DS1307_lastram) return (1);  // fail because address greater then what chip has to read from
    if (addr < 0 ) return (1);     // fail because address less then 0 is not available
    if (quantity > DS1307_lastreg) return (1);  // fail because quantity greater then what can be read
    if ((addr + quantity) > DS1307_lastreg ) return (1);    // fail because cant read past reg 63
    if ( quantity == 0 ) return (1);    // fail because zero quantity wanted
    temp_data[0] = DS1307_lastreg ;  // note this ram location is used to set the addressing pointer in DS1307
    temp_data[1] = 0;    // just junk to clober this address with
    test = ds1307i2c.write(DS1307_addr,temp_data,2);
    if (test == 1) return (1);  // the write operation failed
    ds1307i2c.stop();   // now the DS1307 is pointing to the first register
    if ( addr != 0 ) test = ds1307i2c.read(DS1307_addr,temp_data,addr);  // now the DS1307 address pointer is pointing to correct address
    if (test == 1) return (1);  // the read operation failed
    test = ds1307i2c.read(DS1307_addr,data,quantity);   // read the DS1307 registers now
    if (test == 1) return (1);  // read operation failed
    return(0);  // looks like the data read was good
}
 
int DS1307::read(int addr, int *data) {
    // addr must be 0 - 62 as the 64th(or 63rd as indexed from 0) ram location is clobered in this method of access
    int test = 0;
    char temp_data[65];
    test = DS1307::read(addr, 1, &temp_data[0]);
    if (test == 1) return(1);   // fail because read to DS1307 failed
    *data = (int)temp_data[0];      // returing the read data by pointer
    return (0);                     // the single read is successfull
}
 
int DS1307::write( int addr, int quantity, char *data) {
    // note the char array at data must contain 63 locations or unpredictable behavior will happen
    // addr must be 0 - 62 as the 64th(or 63rd as indexed from 0) ram location is clobered in this method of access
    // quantity must be 1 - 63 as the 64th ram location is clobered in this method of access
    int test = 0 ;
    char temp_data[65] ;
    int loop = 0;
 
    if (addr > DS1307_lastram) return (1);      // fail because address greater then what chip has to read from
    if (addr < 0 ) return (1);      // fail because address less then 0 is not available
    if (quantity > DS1307_lastreg) return (1);  // fail because quantity greater then what can be read
    if (quantity == 0) return (1);  // fail because zero quantity is wanted
    if ((addr + quantity) > DS1307_lastreg ) return (1);    // fail because cant read past reg 63
 
    temp_data[0] = (char)addr;
    for ( ; loop < quantity ; loop++ ) {
        temp_data[loop+1] = *(data + loop);
    }
    test = ds1307i2c.write(DS1307_addr, temp_data, (quantity + 1));
    ds1307i2c.stop();
    return(test);   // 0 for success 1 for failure to write
}
 
int DS1307::write( int addr, int data ) {
    // addr must be 0 - 62 as the 64th(or 63rd as indexed from 0) ram location is clobered in this method of access
    int test = 0 ;
    char temp_data[2] ;
 
    temp_data[0] = (char)addr;
    temp_data[1] = (char)data;
    if (addr > DS1307_lastram) return (1);      // fail because address greater then what chip has to read from
    if (addr < 0 ) return (1);      // fail because address less then 0 is not available
    test = ds1307i2c.write(DS1307_addr, temp_data, 2);
    ds1307i2c.stop();
    return(test);
}
 
int DS1307::start_clock(void) {           // start the clock
    int test = 0;
    int junk = 0;
 
    test = DS1307::read(DS1307_sec, &junk);
    if (test == 1) return(1);       // fail because read to DS1307 failed
    junk = ( 0x7F & junk);              // basicaly i mask bit 8 to set it to zero
    test = DS1307::write(DS1307_sec,junk);        // now write the seconds back to register and because bit 8 is zero this starts clock.
    if (test == 1) return(1);        // fail because read to DS1307 failed
    return(test);                        //
}
 
int DS1307::stop_clock(void) {           // stop clock
    int test = 0;
    int junk = 0;
 
    test = DS1307::read(DS1307_sec, &junk);
    if (test == 1) return(1);        // fail because read to DS1307 failed
    junk = ( 0x7F & junk);              // basicaly i mask bit 8 to set it to zero but keep all other bits
    junk = ( 0x80 | junk);              // basicaly i mask bit 8 to set it to one
    test = DS1307::write(DS1307_sec,junk);        // now write the seconds back to register and because bit 8 is one this starts clock.
    if (test == 1) return(1);        // fail because read to DS1307 failed
    return(test);                        //
}
 
int DS1307::twelve_hour(void) {           // set 12 hour mode
    int test = 0;
    int junk = 0;
 
    test = DS1307::read(DS1307_hour, &junk);
    if (test == 1) return(1);       // fail because read to DS1307 failed
    if ((junk & 0x40) == 0x40) return(0);         // return because 12 mode is active now all done!
 
    junk = ( junk & 0x3F);              // only use 24 hour time values
    if (junk == 0x00)
        junk = 0x12;
    else if (junk >= 0x13)
        if (junk < 0x20) {
            junk = junk - 0x12;
            junk = (junk | 0x20);   // add back the pm indicator
        } else
            switch (junk) {
                case 0x20:
                    junk = 0x28;
                    break;
                case 0x21:
                    junk = 0x29;
                    break;
                case 0x22:
                    junk = 0x30;
                    break;
                case 0x23:
                    junk = 0x31;
                    break;
            }
 
    test = DS1307::write(DS1307_hour,(0x40 | junk));    // set bit 6 with the new 12 hour time converted from the 24 hour time
    if (test == 1) return(1);       // fail because read to DS1307 failed
 
    return(0);
}
 
int DS1307::twentyfour_hour(void) {     // set 24 hour mode
    int test = 0;
    int junk = 0;
 
    test = DS1307::read(DS1307_hour, &junk);
    if (test == 1) return(1);       // fail because read to DS1307 failed
    if ((junk & 0x40) == 0) return(0);         // return because 24 mode is active now all done!
 
    junk = (junk &  0xBF);              // get value bits and am/pm indicator bit but drop 12/24 hour bit
 
    if (junk > 0x12)
        if ( junk <= 0x27 )
            junk = junk - 0x0E;
        else
            junk = junk - 0x08;
 
    test = DS1307::write(DS1307_hour,( 0xBF & junk));   // clear bit 6 and set the new 24 hour time converted from 12 hour time
    if (test == 1) return(1);       // fail because read to DS1307 failed
 
    return(0);
}
 
int DS1307::settime(int sec, int min, int hour, int day, int date, int month, int year) {        // to set the current time and start clock
    // sec = 0 to 59, min = 0 to 59, hours = 0 to 23 ( 24 hour mode only ), day = 1 to 7 ( day of week ), date = 1 to 31, month = 1 to 12, year 0 to 99 ( this is for 2000 to 2099)
    DS1307::stop_clock();
 
    if (1 == DS1307::hilow_check( 59, 0, sec)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_sec,DS1307::dectobcd(sec)))) return(1);    // failed to write for some reason
    }
 
    if (1 == DS1307::hilow_check( 59, 0, min)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_min,DS1307::dectobcd(min)))) return(1);    // failed to write for some reason
    }
 
    if (1 == DS1307::twentyfour_hour()) return(1);                                      // failed to set 24 hour format
    if (1 == DS1307::hilow_check( 23, 0, hour)) {                                       // note setting 24 hour mode befor and after writing the hour value ensures 24 hour mode is set
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_hour,DS1307::dectobcd(hour)))) return(1);    // failed to write for some reason
    }
    if (1 == DS1307::twentyfour_hour()) return(1);                                      // failed to set 24 hour format
 
    if (1 == DS1307::hilow_check( 7, 1, day)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_day,DS1307::dectobcd(day)))) return(1);    // failed to write for some reason
    }
 
    if (1 == DS1307::hilow_check( 31, 1, date)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_date,DS1307::dectobcd(date)))) return(1);    // failed to write for some reason
    }
 
    if (1 == DS1307::hilow_check( 12, 1, month)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_month,DS1307::dectobcd(month)))) return(1);    // failed to write for some reason
    }
 
    if (1 == DS1307::hilow_check( 99, 0, year)) {
        return(1);    // failed because recieved value is not in bounds
    } else {
        if (1 == (DS1307::write(DS1307_year,DS1307::dectobcd(year)))) return(1);    // failed to write for some reason
    }
 
    DS1307::start_clock();
    return (0);             // time is now set
}
 
int DS1307::gettime(int *sec, int *min, int *hour, int *day, int *date, int *month, int *year) { // to get the current time information
    // sec = 0 to 59, min = 0 to 59, hours = 0 to 23 ( 24 hour mode only ), day = 1 to 7 ( day of week ), date = 1 to 31, month = 1 to 12, year 0 to 99 ( this is for 2000 to 2099)
    if (1 == DS1307::read(DS1307_sec,sec)) return(1);       // failed to read for some reason
    *sec = (*sec & 0x7F );                                  // drop the clock start stop bit
    *sec = DS1307::bcdtodec( *sec);                         // bcd is now dec value
 
    if (1 == DS1307::read(DS1307_min,min)) return(1);       // failed to read for some reason
    *min = (*min & 0x7F );                                  // drop bit 7 because it should be 0 anyways
    *min = DS1307::bcdtodec( *min);                         // bcd is now dec value
 
    if (1 == DS1307::read(DS1307_hour,hour)) return(1);     // failed to read for some reason
    if ((*hour & 0x40) == 0x40) {                           // if true then 12 hour mode is set currently  so change to 24 hour, read value, and return to 12 hour mode
        if (1 == DS1307::twentyfour_hour()) return(1);          // failed to set 24 hour mode for some reason
        if (1 == DS1307::read(DS1307_hour,hour)) return(1);     // failed to read for some reason
        *hour = (*hour & 0x3F );                                // drop bit 7 & 6 they are not used for 24 hour mode reading
        *hour = DS1307::bcdtodec( *hour);                       // bcd is now dec value
        if (1 == DS1307::twelve_hour()) return(1);              // failed to return to 12 hour mode for some reason
    } else {                                                     // in 24 hour mode already so just read the hour value
        if (1 == DS1307::read(DS1307_hour,hour)) return(1);    // failed to read for some reason
        *hour = (*hour & 0x3F );                                // drop bit 7 & 6 they are not used for 24 hour mode reading
        *hour = DS1307::bcdtodec( *hour);                       // bcd is now dec value
    }
 
    if (1 == DS1307::read(DS1307_day,day)) return(1);       // failed to read for some reason
    *day = (*day & 0x07 );                                  // drop the non used bits
    *day = DS1307::bcdtodec( *day);                         // bcd is now dec value
 
    if (1 == DS1307::read(DS1307_date,date)) return(1);     // failed to read for some reason
    *date = (*date & 0x3F );                                // drop bit 6 and 7 not used for date value
    *date = DS1307::bcdtodec( *date);                       // bcd is now dec value
 
    if (1 == DS1307::read(DS1307_month,month)) return(1);   // failed to read for some reason
    *month = (*month & 0x1F );                              // drop bit 5, 6 and 7 not used for month value
    *month = DS1307::bcdtodec( *month);                     // bcd is now dec value
 
    if (1 == DS1307::read(DS1307_year,year)) return(1);     // failed to read for some reason
    *year = DS1307::bcdtodec( *year);                       // bcd is now dec value
 
    return (0);                                             // data returned is valid
}
 
 
int DS1307::dectobcd( int dec) {
    int low = 0;
    int high = 0;
 
    high = dec / 10;                // this gives the high nibble value
    low = dec - (high * 10);        // this gives the lower nibble value
    return ((high *16) + low);      // this is the final bcd value but in interger format
}
 
int DS1307::bcdtodec( int bcd) {
    int low = 0;
    int high = 0;
 
    high = bcd / 16;
    low = bcd - (high * 16);
    return ((high * 10) + low);
 
}
 
int DS1307::hilow_check( int hi, int low, int value) {
    if ((value >= low)&(value <= hi))
        return(0);              // value is equal to or inbetween hi and low
    else
        return(1);              // value is not equal to or inbetween hi and low
}