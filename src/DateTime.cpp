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

#include "DateTime.h"
#include "DS3231RTC.h"
// Utilities from JeeLabs/Ladyada

////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed



/***************************************** 
	Public Functions
 *****************************************/

/*******************************************************************************
 * TO GET ALL DATE/TIME INFORMATION AT ONCE AND AVOID THE CHANCE OF ROLLOVER
 * DateTime implementation spliced in here from Jean-Claude Wippler's (JeeLabs)
 * RTClib, as modified by Limor Fried (Ladyada); source code at:
 * https://github.com/adafruit/RTClib
 ******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime (time_t t) {
    static int daysPerMonths[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int y = 0;
    int m =0;
    int d =0;
    int ss = t % 60;
    t /= 60;
    int mm = t % 60;
    t /= 60;
    int hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (y = 0; ; ++y) {
        leap = (y+2) % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = daysPerMonths[m-1];
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1; 
    year = y+1970;
    month = m;
    day = d;
    hour = hh;
    minute = mm;
    second = ss;
}

DateTime::DateTime (uint16_t y, uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss) {
    year = y;
    month = m;
    day = d;
    hour = hh;
    minute = mm;
    second = ss;
}

DateTime::DateTime (const char* datetime)
{
    uint8_t buf[6] = {1900,1,1,0,0,0};
    char str[32];
    strncpy(str,datetime,32);
    char del[] = ":/- ";
    char * tok= strtok(str,del);
    int i = 0;
    bool yearFirst = false;
    while(i<6 && tok){      
        if(i==0 && datetime[strlen(tok)] == ':')i=3;
        int v = atol(tok);            
        if(i==0 && v>99) yearFirst = true;
        if(v>99)v=v%2000;
        buf[i++] = v;
        tok = strtok(NULL,del);
    }

    if(yearFirst){
        this->year = buf[0]+2000;
        this->month = buf[1];
        this->day = buf[2];
    }else{
        this->year = buf[2]+2000;
        this->month = buf[1];
        this->day = buf[0];    
    }
    this->hour = buf[3];
    this->minute = buf[4];
    this->second = buf[5];

}

byte DateTime::dayOfWeek() {   // y > 1752, 1 <= m <= 12
  int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};  
  year -= month < 3;
  return ((year + year/4 - year/100 + year/400 + t[month-1] + day) % 7) + 1; // 01 - 07, 01 = Sunday
}

 DateTime:: operator time_t(){
    static int monthdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int yd = year-1970;
    long days = ((yd)*365) + ((yd+2)/4) + (day-1);
    for(int i = 0;i<month;i++) days+=monthdays[i];
    return (time_t) (((days * 24L + hour) * 60 + minute) * 60 + second);
}
/////////////////////////////////////////////////////////////////////
