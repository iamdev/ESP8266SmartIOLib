#define BLYNK_PRINT Serial

#include <SerialDebug.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266SmartIOLib.h>
#include <PCF8574_IOEXP.h>
#include <SimpleTask.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleDHT.h>
#include "config.h"

/*--Hardware List--
1. LCD-1602 I2C
2. DHT22 Connect to Pin D5
------------------*/

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
PCF8574 ex(PCF8574_I2C_ADDR);
EEPROM_EX eeprom(EEPROM_I2C_ADDR); 
DeviceClass Device(DEVICE_SIGNATURE);
DS3231RTC rtc;
SerialCommand cmd;
LiquidCrystal_I2C lcd(0x3F,16,2);
SimpleDHT22 dht(D5);
BlynkTimer timer;
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
float temperature;
float humidity;
/******************************************/


/******************************************/
/* Function Definition                    */
/******************************************/
void handleWebConfig();
void setupTasks();
void readExpInput();
void updateDisplay(task_t &tk);
void readSensor();
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
  delay(1000);

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

  /******************************************/
  /* Setup PCF8574 IO-Expander              */
  /******************************************/  
  ex.setOutputChannels(4,exp_output_pin);
  ex.setInputChannels(4,exp_input_pin);
  ex.begin();
  ex.onInputChange(readExpInput);
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
  //timer.run();
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
#if AUTO_UPDATE>0 
  wc.addConfigField("updateServer","Update Server",(const char*)AppSetting.updateServer);
#endif
  wc.handleCallback([](JsonObject doc){      
      SerialDebug("WebConfig Callback :\n");
      SerialDebug_printf("blynkServer = %s",(const char *)doc["blynkServer"]);
      SerialDebug_printf("blynkPort = %s",(const char *)doc["blynkPort"]);
      SerialDebug_printf("blynkToken = %s",(const char *)doc["blynkToken"]);
      strncpy(AppSetting.blynkServer,(const char *)doc["blynkServer"],sizeof(AppSetting.blynkServer));
      AppSetting.blynkPort = atol((const char *)doc["blynkPort"]);
      strncpy(AppSetting.blynkToken,(const char *)doc["blynkToken"],sizeof(AppSetting.blynkToken));
      #if AUTO_UPDATE > 0  
      strncpy(AppSetting.updateServer,(const char *)doc["updateServer"]);
      #endif
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
  Task.create(readSensor,1000);
  /* Task : Update Diplay */
  Task.create(updateDisplay,200);
  Task.create(connectBlynk,0);
  dht.read2(&temperature, &humidity, NULL);
}

void readExpInput(){
    for(int i=0;i<4;i++){
      struct InputStatus s = ex.getInputStatus(i);
      if(s.state == INPUT_STATE_RELEASED){        
        int n = ex.toggle(i);
        switch(i){
          case 0:Blynk.virtualWrite(V0,!n);break;
          case 1:Blynk.virtualWrite(V1,!n);break;
          case 2:Blynk.virtualWrite(V2,!n);break;
          case 3:Blynk.virtualWrite(V3,!n);break;
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
#if AUTO_UPDATE >0  
  SerialDebug_printf("Update Server Address :%s\n",AppSetting.updateServer);
#endif  
  SerialDebug("-------------------------------\n");
}

void updateDisplay(task_t &tk){
  if(tk.timeoffset%1000<200){
    lcd.setCursor(0,0);
    DateTime t = rtc.now();
    char str[17]; 
    snprintf(str,17,"%02u/%02u/%4u %02u:%02u",t.day,t.month,t.year,t.hour,t.minute);
    lcd.print(str);
    snprintf(str,17,"T:%2.1f%cC H:%2.1f%c",temperature,223,humidity,'%');
    lcd.setCursor(0,1);
    lcd.print(str);
  }else if(tk.timeoffset%1000>=800){
    lcd.setCursor(13,0);
    lcd.print(" ");
  }
}

void readSensor(){
    int err = SimpleDHTErrSuccess;
    if ((err = dht.read2(&temperature, &humidity, NULL)) == SimpleDHTErrSuccess) {
      if(humidity>=100)humidity=99.9f;
      Blynk.virtualWrite(V4,String(temperature, 1).c_str());
      Blynk.virtualWrite(V5,String(humidity, 1).c_str());      
    }else{
      SerialDebug_printf("Read DHT failed, err=%d\n",err);   
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


/******************************************/
