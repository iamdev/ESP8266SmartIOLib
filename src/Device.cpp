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

#include "Device.h"

#include "EEPROM_EX.h"
#include "ESP8266TrueRandom.h"

DeviceClass::DeviceClass(unsigned char signature):_signature(signature){
}

void DeviceClass::init(){
    delay(100);
    unsigned char sig = eeprom.read(DEVICE_DATA_SIGNATURE_ADDR);
    if(sig != _signature){
        Device.format();
    }else{
        eeprom.read(DEVICE_DATA_SERIALNO_ADDR,&this->serialno,sizeof(this->serialno));
    }
    this->load();
}

void DeviceClass::load(){
    eeprom.read(DEVICE_DATA_CONFIG_ADDR,&wifiConfig,sizeof(WiFiConfig));
}

void DeviceClass::save(){
    eeprom.write(DEVICE_DATA_CONFIG_ADDR,&wifiConfig,sizeof(WiFiConfig));
}

void DeviceClass::format(){
    this->serialno = DEVICE_ID_PREFIX + ESP8266TrueRandom.random(DEVICE_ID_MIN,DEVICE_ID_MAX);
    this->wifiConfig = {};
    this->save();
    eeprom.write(DEVICE_DATA_SERIALNO_ADDR,&(this->serialno),sizeof(this->serialno));    
    eeprom.write(DEVICE_DATA_SIGNATURE_ADDR,_signature);
    if(this->onFormatCallback)this->onFormatCallback();
}

void DeviceClass::onFormat(void(*callback)(void)){
    this->onFormatCallback = callback;
}
