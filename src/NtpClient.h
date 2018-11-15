#ifndef _NTP_CLIENT_H
#define _NTP_CLIENT_H
#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>

#define NTPCLIENT_DEFAULT_PORT      123
#define NTPCLIENT_DEFAULT_SERVER    "time.nist.gov"
#define NTP_PACKET_SIZE             48
#define NTPCLIENT_DEFAULT_TIMEOUT   5000
#define NTP_DEFAULT_TIMEZONE        7
class NtpClient{
    public :
        NtpClient();
        NtpClient(const char * ntpServer,int localport=NTPCLIENT_DEFAULT_PORT);
        void begin();
        time_t getTime(int timezone=NTP_DEFAULT_TIMEZONE,int timeout=NTPCLIENT_DEFAULT_TIMEOUT);
    private:
        boolean sendNTPpacket ();
        char _ntpServer[100];
        int _ntpLocalPort;
        WiFiUDP *udp;
        int timeZone=NTP_DEFAULT_TIMEZONE;
};

#endif