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
