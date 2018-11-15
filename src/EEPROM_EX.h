#ifndef _EEPROM_H
#define _EEPROM_H

#define EEPROM_WRITE_PAGE 1
#define EEPROM_RW_DELAY 10
#define EEPROM_PAGE_SIZE 64
#define EEPROM_BLOCK_SIZE 16
#include <Wire.h>

class EEPROM_EX{
public :
    EEPROM_EX(TwoWire *wire,int deviceAddress);
    EEPROM_EX(int deviceAddress=0x50); 
    bool isValid();
    unsigned char read(unsigned int address);
    void read(unsigned int address,void *buffer,int length);
    void write(unsigned int address,unsigned char data);
    void write(unsigned int address,void *buffer,int length);
private:
    int _deviceAddress;
    bool _isValid;
    TwoWire *_wire;
};
extern EEPROM_EX eeprom;
#endif
