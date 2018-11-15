#ifndef _AUTOUPDATE_H
#define _AUTOUPDATE_H
#include <Arduino.h>

//version.txt contain version number => x.y.z
//ex. 1.0.2
#define UPDATE_URL_VERSION_PATH "/firmware/version.txt" 
#define UPDATE_URL_BIN_PATH "/firmware/firmware.bin"

struct version_t{
    uint8_t major;
    uint8_t minor;
    uint16_t build;
};
bool autoUpdate(const char*serverAddress,
                const char * currentVersion,
                const char * versionPath=UPDATE_URL_VERSION_PATH, 
                const char * firmwarePath=UPDATE_URL_BIN_PATH);

#endif
