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
