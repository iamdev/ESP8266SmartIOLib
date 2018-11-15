#include "DS3231RTC.h"
#include <Arduino.h>

static uint8_t dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

static uint8_t bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

static uint8_t inp2toi(char *cmd, const uint16_t seek)
{
    uint8_t rv;
    rv = (cmd[seek] - 48) * 10 + cmd[seek + 1] - 48;
    return rv;
}

DS3231RTC::DS3231RTC(TwoWire *wire)
    :_wire(wire)
{    
}

DS3231RTC::DS3231RTC()
    :DS3231RTC(&Wire)
{
}

void DS3231RTC::set_addr(const uint8_t addr, const uint8_t val)
{
    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(addr);
    _wire->write(val);
    _wire->endTransmission();
}

uint8_t DS3231RTC::get_addr(const uint8_t addr)
{
    uint8_t rv;

    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(addr);
    _wire->endTransmission();

    _wire->requestFrom(DS3231_I2C_ADDR, 1);
    rv = _wire->read();

    return rv;
}

void DS3231RTC::set_creg(const uint8_t val)
{
    set_addr(DS3231_CONTROL_ADDR, val);
}

void DS3231RTC::set_sreg(const uint8_t val)
{
    set_addr(DS3231_STATUS_ADDR, val);
}

uint8_t DS3231RTC::get_sreg(void)
{
    uint8_t rv;
    rv = get_addr(DS3231_STATUS_ADDR);
    return rv;
}

float DS3231RTC::getTemperature()
{
    float rv;
    uint8_t temp_msb, temp_lsb;
    int8_t nint;

    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(DS3231_TEMPERATURE_ADDR);
    _wire->endTransmission();

    _wire->requestFrom(DS3231_I2C_ADDR, 2);
    temp_msb = _wire->read();
    temp_lsb = _wire->read() >> 6;

    if ((temp_msb & 0x80) != 0)
        nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
    else
        nint = temp_msb;
    rv = 0.25 * temp_lsb + nint;
    return rv;
}

struct DateTime DS3231RTC::now()
{
    uint8_t TimeDate[7];        //second,minute,hour,dow,day,month,year
    uint8_t century = 0;
    uint8_t i, n;
    uint16_t year_full;
    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(DS3231_TIME_CAL_ADDR);
    _wire->endTransmission();

    _wire->requestFrom(DS3231_I2C_ADDR, 7);

    for (i = 0; i <= 6; i++) {
        n = _wire->read();
        if (i == 5) {
            TimeDate[5] = bcdtodec(n & 0x1F);
            century = (n & 0x80) >> 7;
        } else
            TimeDate[i] = bcdtodec(n);
    }

    if (century == 1) {
        year_full = 2000 + TimeDate[6];
    } else {
        year_full = 1900 + TimeDate[6];
    }
    return DateTime(year_full,TimeDate[5],TimeDate[4],TimeDate[2],TimeDate[1],TimeDate[0]);
}

void DS3231RTC::set(time_t t)
{
    set(DateTime(t));        
} 

void DS3231RTC::set(DateTime t)
{ 
    uint8_t i, century; 
    uint8_t short_year;
    if (t.year >= 2000) {
        century = 0x80;
        short_year = t.year - 2000;
    } else {
        century = 0;
        short_year = t.year - 1900;
    }
    uint8_t TimeDate[7] = { t.second, t.minute, t.hour, t.dayOfWeek(), t.day, t.month,short_year };
    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(DS3231_TIME_CAL_ADDR);
    for (i = 0; i <= 6; i++) {
        TimeDate[i] = dectobcd(TimeDate[i]);
        if (i == 5)
            TimeDate[5] += century;
        _wire->write(TimeDate[i]);
    }
    _wire->endTransmission();
} 

void DS3231RTC::setAlarmInterrupt(DateTime t){
    this->clearAlarmInterrupt();
    _wire->beginTransmission(DS3231_I2C_ADDR);
    _wire->write(DS3231_ALARM1_ADDR);
    _wire->write(dectobcd(t.second));
    _wire->write(0x80|dectobcd(t.minute));
    _wire->write(0x80|dectobcd(t.hour));
    _wire->write(0x80);   
    _wire->endTransmission();     
    this->set_creg(DS3231_INTCN|DS3231_A1IE);
}

void DS3231RTC::clearAlarmInterrupt(){
  this->set_addr(DS3231_STATUS_ADDR,(this->get_addr(DS3231_STATUS_ADDR) & ~DS3231_A1IE)); //Clear alarm flag && Interrupt
  this->set_creg(DS3231_INTCN & ~DS3231_A1IE);
}

void DS3231RTC::attachAlarmInterrupt(uint8_t pin,void(*callback)(void)){
    this->clearAlarmInterrupt();
    pinMode(pin,INPUT_PULLUP);
    attachInterrupt(pin, callback, FALLING);  
}

time_t now(){
    return (time_t)rtc.now(); 
}