
#define AUTO_UPDATE     0

#ifndef SDA 
#define SDA     4
#endif 

#ifndef SCL 
#define SCL     5
#endif 

#ifdef LED_BUILTIN
#undef LED_BUILTIN
#endif
#define LED_BUILTIN                 2

#define EEPROM_I2C_ADDR             0x50
#define EEPROM_SIZE                 0x8000
#define EEPROM_SETTING_ADDR         256
#define DEVICE_SIGNATURE            0x1A

#define PCF8574_I2C_ADDR            0x21
#define PCF8574_INT_PIN             -1

#define DS3231_I2C_ADDR             0x68
#define RTC_INT_PIN                 -1

#define I2C_SDA                     SDA
#define I2C_SCL                     SCL
#define BUTTON_PIN                  16

#define AP_WEBCONFIG_NAME           "ESP-"
#define AP_WEBCONFIG_PASSWORD       "88888888"

#define WIFI_CONNECTION_TIMEOUT     10000

static char ntpServer[100] = "pool.ntp.org";
//static int ntpTimeZone = 7 
//static int minutesTimeZone = 0;
#define ntpTimeZone 7
#define minutesTimeZone 0

const int exp_input_pin[] = {4,5,6,7};
const int exp_output_pin[] = {0,1,2,3};
