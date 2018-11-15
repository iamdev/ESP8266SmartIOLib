
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
