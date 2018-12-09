/*******************************************************************/
// This example compatible for MakeArduino Smart I/O Shield for WeMos D1 mini
// Features:
//   1. Input push switch 8ch
//   2. Output 8ch (Active Low) to connect Relay Module or other 
//   3. LCD Display Clock
//   4. WiFi SSID/Pass via APWebConfig (Captive portal) while logon wifi 
//      ESP-xxxxxxxx with pass 88888888
//   5. Support Blynk application config via APWebConfig
//   6. Output interface with Blynk via V0-V7
/*******************************************************************/
// Hardware Required :
//   1. LCD 1602 with I2C interface
//   2. 8ch Relay modules
//   3. 8push button or membrain keypad
/*******************************************************************/
// External Library Required :
//   1. Blynk - Install from Library Manager
//   2. LiquidCrystal_I2C (by Frank de Brabander) Install from Library Manager
//   3. SimpleTask - Download and install from Zip 
//      (https://github.com/iamdev/SimpleTask/releases/latest)
/*******************************************************************/
// สนใจสั่งซื้อบอร์ด SmartIO Shield 16ch ได้ที่ fb:makearduino
/*******************************************************************/

#define BLYNK_PRINT Serial

#include <SerialDebug.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266SmartIOLib.h>
#include <PCA9539_IOEXP.h>
#include <SimpleTask.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

/******************************************/
/* Global Type Definition                 */
/******************************************/
struct AppSetting_t{
  char blynkServer[50];
  int blynkPort;
  char blynkToken[33];
}AppSetting;
/******************************************/

/******************************************/
/* Global Class Instant                   */
/******************************************/
PCA9539 ex(PCA9539_I2C_ADDR);
EEPROM_EX eeprom(EEPROM_I2C_ADDR); 
DeviceClass Device(DEVICE_SIGNATURE);
DS3231RTC rtc;
SerialCommand cmd;
LiquidCrystal_I2C lcd(0x3F,16,2);
//LiquidCrystal_I2C lcd(0x27,16,2);
/******************************************/ 

#ifdef DEBUG
void printSetting();
#else
#define printSetting()
#endif

/******************************************/
/* User Global Variables                  */
/******************************************/
bool ntpSuccess = false;
/******************************************/


/******************************************/
/* Function Definition                    */
/******************************************/
void handleWebConfig();
void setupTasks();
void readExpInput();
void updateDisplay(task_t &tk);
void connectBlynk(task_t &tk);
/******************************************/

void setup() {
#ifdef DEBUG
  Serial.begin(115200);  
  delay(100); 
  SerialDebug("\n");
#endif
  SerialDebug("\n");
  Wire.begin(I2C_SDA,I2C_SCL);

  /******************************************/
  /* Setup PCF8574 IO-Expander              */
  /******************************************/  
  ex.setOutputChannels(8,exp_output_pin);
  ex.setInputChannels(8,exp_input_pin);
  ex.onInputChange(readExpInput);
  ex.begin();
  ex.write(0xFF);
  /******************************************/
  
  /******************************************/
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Initializing...");
  /******************************************/

  /******************************************/
  /* Initalize Device and load Setting Data */
  /******************************************/
  eeprom.read(EEPROM_SETTING_ADDR,&AppSetting,sizeof(AppSetting_t));
  Device.onFormat([]{
    AppSetting={};
    strncpy(AppSetting.blynkServer,String("blynk-cloud.com").c_str(),sizeof(AppSetting.blynkServer));
    AppSetting.blynkPort = 80;
    eeprom.write(EEPROM_SETTING_ADDR,&AppSetting,sizeof(AppSetting_t));
  });
  Device.init();
  /******************************************/

  SerialDebug_printf("Device serial number :%d\n",Device.SerialNumber()); 
  SerialDebug("--------------------\n");  

  /******************************************/
  /* check button to enter AP WebConfig mode*/
  /******************************************/ 
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  if(!digitalRead(BUTTON_PIN)){    
    pinMode(LED_BUILTIN,OUTPUT); 
    digitalWrite(LED_BUILTIN,LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Config Mode...");
    handleWebConfig();    
  }
  /******************************************/
  printSetting();
  /******************************************/
  /* Connect WiFi with timeout aboart (10s)  */
  /******************************************/
  SerialDebug("Connecting to WiFi.\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin(Device.wifiConfig.ssid,Device.wifiConfig.pass);
  {
    long _ms = millis();
    while (WiFi.status() != WL_CONNECTED && millis()<(_ms+WIFI_CONNECTION_TIMEOUT)) {
      delay(200);
      SerialDebug("|");
    }
  }
  SerialDebug("\n");
  /******************************************/

  #ifdef DEBUG
  delay(500);
  SerialDebug("WiFi Diagnostics :\n");
  SerialDebug("-------------------------------\n");
  WiFi.printDiag(Serial);
  SerialDebug("-------------------------------\n");
  #endif  
  SerialDebug_printf("Device IP   : %s\n",WiFi.localIP().toString().c_str());
  SerialDebug_printf("Subnet Mask : %s\n",WiFi.subnetMask().toString().c_str());   
  SerialDebug_printf("Gateway IP  : %s\n",WiFi.gatewayIP().toString().c_str());
    
  /******************************************/
  /* NTP Server - RTC synchronize           */
  /******************************************/
  if(WiFi.status() == WL_CONNECTED)
  {
    SerialDebug("--------------------\n");
    SerialDebug_printf("Start NTP Sync...");
    NtpClient ntp(ntpServer);
    ntp.begin();
    time_t tt = ntp.getTime();
    if(tt>0){
      SerialDebug_printf("success.\n");
      SerialDebug_printf("Unix Time : %u\n",(long)tt);
      rtc.set(tt);
    }else{
      SerialDebug_printf("timeout.\n");
    }
    SerialDebug("--------------------\n");
  }
  DateTime t = rtc.now();
  SerialDebug_printf("Current DateTime : %02u/%02u/%4u %02u:%02u:%02u \n",t.day,t.month,t.year,t.hour,t.minute,t.second);
  /******************************************/

  
  /******************************************/
  /* Blynk Setup                            */
  /******************************************/
  //Blynk.begin(
  //  AppSetting.blynkToken,
  //  Device.wifiConfig.ssid,
  //  Device.wifiConfig.pass,
  //  AppSetting.blynkServer,
  //  AppSetting.blynkPort);
  Blynk.config(AppSetting.blynkToken,AppSetting.blynkServer, AppSetting.blynkPort);
  /******************************************/

  /******************************************/
  /* Create Task (loop sequential task)     */
  /******************************************/
  setupTasks();
  //timer.setInterval(1, [](){Task.loop();});

  /******************************************/

  SerialDebug("--------------------\n");
  SerialDebug("Setup Completed.\n");
  SerialDebug("--------------------\n");   
}

void loop() {
  Task.loop();
  if(Blynk.connected())Blynk.run();
}

void handleWebConfig(){
  String ssid = AP_WEBCONFIG_NAME;
  ssid += Device.SerialNumber();
  WebConfig wc(ssid.c_str(),AP_WEBCONFIG_PASSWORD);
  wc.begin();
  String port = String(AppSetting.blynkPort,DEC);
  wc.addConfigField("blynkServer","Blynk Server",(const char*)AppSetting.blynkServer);
  wc.addConfigField("blynkPort","Blynk Port",port.c_str());
  wc.addConfigField("blynkToken","Blynk Token",(const char*)AppSetting.blynkToken);
  wc.handleCallback([](JsonObject doc){      
      SerialDebug("WebConfig Callback :\n");
      SerialDebug_printf("blynkServer = %s",(const char *)doc["blynkServer"]);
      SerialDebug_printf("blynkPort = %s",(const char *)doc["blynkPort"]);
      SerialDebug_printf("blynkToken = %s",(const char *)doc["blynkToken"]);
      strncpy(AppSetting.blynkServer,(const char *)doc["blynkServer"],sizeof(AppSetting.blynkServer));
      AppSetting.blynkPort = atol((const char *)doc["blynkPort"]);
      strncpy(AppSetting.blynkToken,(const char *)doc["blynkToken"],sizeof(AppSetting.blynkToken));
      printSetting();
      eeprom.write(EEPROM_SETTING_ADDR,&AppSetting,sizeof(AppSetting_t));
  });
  while(wc.run());
  delay(3000);
  ESP.restart();
}

void setupTasks(){
  /* Task : LED Blink */
  pinMode(LED_BUILTIN,OUTPUT);
  Task.create([](task_t &tk){    
    if(tk.timeoffset%1000<800 && !digitalRead(LED_BUILTIN))
      digitalWrite(LED_BUILTIN,HIGH);
    else if(tk.timeoffset%1000>=800 && digitalRead(LED_BUILTIN))
      digitalWrite(LED_BUILTIN,LOW);
  },200);

  /* Task : Check input loop */
  Task.create([](task_t &tk){ex.inputLoop();},20);    
  /* Task : Update Diplay */
  Task.create(updateDisplay,50);
  Task.create(connectBlynk,0);
}

void readExpInput(){
    for(int i=0;i<8;i++){
      struct InputStatus s = ex.getInputStatus(i);
      if(s.state == INPUT_STATE_RELEASED){        
        int n = ex.toggle(i);
        if(i==7 && s.duration > 5000) {
          SerialDebug("Formatting Device...\n");;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Formatting...");
          Device.format();
          delay(2000);
          ESP.restart();
        }
        switch(i){
          case 0:Blynk.virtualWrite(V0,!n);break;
          case 1:Blynk.virtualWrite(V1,!n);break;
          case 2:Blynk.virtualWrite(V2,!n);break;
          case 3:Blynk.virtualWrite(V3,!n);break;
          case 4:Blynk.virtualWrite(V4,!n);break;
          case 5:Blynk.virtualWrite(V5,!n);break;
          case 6:Blynk.virtualWrite(V6,!n);break;
          case 7:Blynk.virtualWrite(V7,!n);break;
          
        }
        SerialDebug_printf("Button %d Pressed!! (%d ms.)\n",i+1,s.duration);
      }
    }
}

void printSetting(){
  SerialDebug("-------------------------------\n");
  SerialDebug("Device Confuguration :\n");
  SerialDebug_printf("Wifi SSID:%s\n",Device.wifiConfig.ssid);
  SerialDebug_printf("Wifi Pass:%s\n",Device.wifiConfig.pass);
  SerialDebug_printf("Blynk Server : %s:%d\n",AppSetting.blynkServer,AppSetting.blynkPort);
  SerialDebug_printf("Blynk Token :%s\n",AppSetting.blynkToken);
  SerialDebug("-------------------------------\n");
}

void updateDisplay(task_t &tk){
  if(tk.timeoffset%1000<50){
    lcd.setCursor(0,0);
    DateTime t = rtc.now();
    char str[17]; 
    snprintf(str,17,"%02u/%02u/%4u %02u:%02u",t.day,t.month,t.year,t.hour,t.minute);
    lcd.print(str);
  }
}

int blynkRetryCount = 0;
void connectBlynk(task_t &tk){
  if(tk.interval==0)tk.interval=5000;
  if(blynkRetryCount>5)tk.interval=60000;
  if(!Blynk.connected() && WiFi.status() == WL_CONNECTED){
    SerialDebug_printf("Reconnect blynk!\n");
    if(Blynk.connect(100))
    {
      blynkRetryCount=0;
    }else{
      blynkRetryCount=0;
      tk.interval=5000;
    }
  }
};


/******************************************/
/* Bynk virtual pin write function        */
/******************************************/
BLYNK_WRITE(V0)
{
  ex.write(0,!param.asInt());
}
BLYNK_WRITE(V1)
{
  ex.write(1,!param.asInt());
}
BLYNK_WRITE(V2)
{
  ex.write(2,!param.asInt());
}
BLYNK_WRITE(V3)
{
  ex.write(3,!param.asInt());
}
BLYNK_WRITE(V4)
{
  ex.write(4,!param.asInt());
}
BLYNK_WRITE(V5)
{
  ex.write(5,!param.asInt());
}
BLYNK_WRITE(V6)
{
  ex.write(6,!param.asInt());
}
BLYNK_WRITE(V7)
{
  ex.write(7,!param.asInt());
}


/******************************************/