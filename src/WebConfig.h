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

#ifndef _WIFI_CONFIG_H
#define _WIFI_CONFIG_H
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

 
#define DEFAULT_AP_SSID "esp_ap_config"
#define DEFAULT_AP_PASSWORD "88888888"
#define AP_DNS_PORT 53

struct field_t{
    const char *name;
    const char *desc;
    const char *value;
};

class WebConfig{
    public :
        WebConfig();
        WebConfig(const char * ap_ssid,const char * ap_password);
        ~WebConfig();
        void begin(bool enableCaptivePortal=true);        
        void addConfigField(const char* name,const char * desc,const char * value);
        void handleCallback(void (*handle) (JsonObject));
        bool run();
    private:
        void handleRoot(void);
        void handleConfig(void);
        void handleInfo();
        bool isIp(String str);
        String toStringIp(IPAddress ip);
        ESP8266WebServer *server;
        DNSServer*dnsServer;
        void (*_handleCallback) (JsonObject);
        bool isCompleted = false;        
        bool isEnableCaptivePortal;
        char ap_ssid[32];
        char ap_password[32];
        int configFieldCount;
        field_t *configFields;
};


#endif
