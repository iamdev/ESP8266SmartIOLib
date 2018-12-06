//***********************************************************************************
// MIT License
// 
// Copyright (c) 2018 Kamon Singtong
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//***********************************************************************************
// Owner : Kamon Singtong (MakeArduino.com)
// email : kamon.dev@hotmail.com
// fb : makearduino
//***********************************************************************************

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
