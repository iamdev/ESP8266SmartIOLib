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

#ifndef _DS3231RTC_H
#define _DS3231RTC_H
#include <Wire.h>
#include "DateTime.h"
#define DS3231_I2C_ADDR             0x68

#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_TEMPERATURE_ADDR     0x11
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F

#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B

// control register bits
#define DS3231_A1IE     0x1
#define DS3231_A2IE     0x2
#define DS3231_INTCN    0x4

class DS3231RTC{
    public:
        DS3231RTC(TwoWire *wire);
        DS3231RTC();
        float getTemperature();
        void set(DateTime t);
        void set(time_t t);
        struct DateTime now();
        void setAlarmInterrupt(DateTime t);
        void clearAlarmInterrupt();
        void attachAlarmInterrupt(uint8_t pin,void(*callback)(void));
    private:
        void set_addr(const uint8_t addr, const uint8_t val);
        uint8_t get_addr(const uint8_t addr);
        void set_creg(const uint8_t val);
        void set_sreg(const uint8_t val);
        uint8_t get_sreg(void);
        uint8_t interruptPin;
        TwoWire *_wire;
};

extern DS3231RTC rtc;
#endif 
