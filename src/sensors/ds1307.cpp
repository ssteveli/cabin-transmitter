#include "ds1307.h"

#define  y2kYearToTm(Y)      ((Y) + 30)

DS1307::DS1307(PinName sda, PinName scl, int address) : m_address(address), ds1307i2c(sda, scl) {
    ds1307i2c.frequency(DS1307_freq);
}
 
DS1307::~DS1307() {
}
 
int DS1307::gettime(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *date, uint8_t *month, uint16_t *year) { 
     char buffer[7] = {0}; 

    int result = ds1307i2c.write(m_address, buffer, 1);
    result = ds1307i2c.read(m_address, buffer, 7);

    if (result != 0) {
        return result;
    }
    
    *sec = bcdtodec(buffer[0] & 0b01111111);
    *min = bcdtodec(buffer[1]);
    *hour = bcdtodec(buffer[2] & 0b00011111);
    *day = bcdtodec(buffer[3]);
    *date = bcdtodec(buffer[4]);
    *month = bcdtodec(buffer[5]);
    *year = y2kYearToTm(bcdtodec(buffer[6]));

    return 0;
}
 
 
uint8_t DS1307::bcdtodec(uint8_t dec) {
    return (dec >> 4) * 10 + (dec & 0b1111);
}
