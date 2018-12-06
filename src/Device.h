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


#ifndef _DEVICE_H
#define _DEVICE_H

#define DEVICE_ID_PREFIX    100e7
#define DEVICE_ID_MIN       1000000
#define DEVICE_ID_MAX       9999999


#define DEVICE_DATA_ADDR        0
#define DEVICE_DATA_SIGNATURE_ADDR   DEVICE_DATA_ADDR 
#define DEVICE_DATA_SERIALNO_ADDR    (DEVICE_DATA_ADDR+1)
#define DEVICE_DATA_CONFIG_ADDR      (DEVICE_DATA_ADDR+5)


class DeviceClass{
    public :
        DeviceClass(unsigned char signature);        
        void format();
        void init();        
        void load();
        void save();
        void onFormat(void(*callback)(void));
        struct WiFiConfig{
            char ssid [64];
            char pass [64];
        } wifiConfig;
        unsigned long SerialNumber() const {return serialno;}        
    private:  
        unsigned long serialno;
        unsigned char _signature;
        void(*onFormatCallback)(void);
}; 

extern DeviceClass Device;
#endif
