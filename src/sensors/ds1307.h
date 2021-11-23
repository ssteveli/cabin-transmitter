#ifndef DS1307_H
#define DS1307_H

#include "mbed.h"
 
#define DS1307_addr 0x68 << 1 // datasheet 8bit 0x68, f401re is 7bit so shift left by 1 0xD0
#define DS1307_freq 100000
 
class DS1307 {
private:
    int m_address = DS1307_addr;

public:
    DS1307(PinName sda, PinName slc, int address = DS1307_addr);
    ~DS1307();
 
    int read(int addr, int length, char *data);
    int gettime(uint8_t *sec, uint8_t *min, uint8_t *hour, uint8_t *day, uint8_t *date, uint8_t *month, uint16_t *year);
 
protected:
    I2C ds1307i2c;
    uint8_t bcdtodec(uint8_t);
};
 
#endif