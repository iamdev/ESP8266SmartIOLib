#include "WebConfig.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include "Device.h"

//#define DEBUG
#include "debug.h"

WebConfig::WebConfig(const char * ap_ssid,const char * ap_password){
  strncpy(this->ap_ssid,ap_ssid,sizeof(this->ap_ssid));
  strncpy(this->ap_password,ap_password,sizeof(this->ap_password));
  configFields = new field_t();
  configFieldCount = 0;
}

WebConfig::WebConfig():WebConfig(DEFAULT_AP_SSID,DEFAULT_AP_PASSWORD){

}

bool WebConfig::run(){
  if(isEnableCaptivePortal){
    this->dnsServer->processNextRequest();
  }
  this->server->handleClient();
  return !isCompleted;
}

void WebConfig::addConfigField(const char* name,const char * desc,const char * value){
    int c = configFieldCount++;
    configFields =  (field_t *) realloc(configFields, configFieldCount * sizeof(field_t));    
    configFields[c].name = name;
    configFields[c].desc = desc;
    configFields[c].value = value;
}

void WebConfig::handleRoot(){
#ifdef DEBUG
  SerialDebug("Config Value:\n");
  for(int i=0;i<configFieldCount;i++){
    SerialDebug_printf("   %s [%s] \t\t\t\t\t\t: %s\n",configFields[i].desc,configFields[i].name,configFields[i].value );
  }
#endif
  String s = "<!DOCTYPE html>";
  s+="<html><head>";
  s+="<meta name='viewport' content='width=device-width, initial-scale=1'>";
  s+="<title>ESP8266 - Smart Device Configuration</title><style>";
  s+="form,[f],[lb],[tx],[fg]{display:flex;flex-direction: row;flex:1 1 auto;font-family:'Comic Sans MS', cursive, sans-serif;font-size:14px;}";
  s+="form{flex-direction: column;max-width:25em;}";
  s+="[f]{height:25px;}";
  s+="[f] [lb]{flex:0 0 8em;}";
  s+="[f] [tx]{flex:1 1 auto;}";
  s+="[fg]{border-radius: 0.5em;padding:0.5em;border:solid 1px #ccc;flex-direction: column;position:relative;padding:1em 1em 1em 0.5em;margin:1em 0 0 0.5em;}";
  s+="[fg-h]{position:absolute;top:-0.5em;padding:0 0.5em;background: white;}";
  s+="input{height:1.3em;}";
  s+="button{width:5em;height:2em;font-family: inherit;}";
  s+="[center]{justify-content: center;align-items: center;}";
  s+="[sep]{margin-top:0.5em;padding:0.5em 0;border-top:solid 1px #ccc;border-bottom:solid 1px #ccc;}";
  s+="</style></head>";
  s+="<body><form method='post' action='/config'><div fg><div fg-h>WiFi-Config</div>";
  s+="<div f><div lb>SSID :</div><div tx><input type='text' name='wifi_ssid' value='";
  s+=Device.wifiConfig.ssid;
  s+="'></div></div>";
  s+="<div f><div lb>Pass :</div><div tx><input type='text' name='wifi_pass' value='";
  s+=Device.wifiConfig.pass;
  s+="'></div></div>";
  s+="</div><div fg><div fg-h>Device Settings</div>";
  s+="<div f><div lb>Serial No :</div><div tx><input type='text' disabled value='";
  s+=Device.SerialNumber();
  s+="'></div></div>";
  for(int i=0;i<configFieldCount;i++){  
    s+="<div f><div lb>";
    s+=configFields[i].desc;
    s+=" :</div><div tx><input name='";
    s+=configFields[i].name;
    s+="' type='text' value='";
    s+=configFields[i].value;
    s+="'></div></div>";
  } 
  s+="</div><div f center sep><button type='submit'>Save</button></div></form></body></html>";  
  server->send(200, "text/html", s);
}

void WebConfig::handleConfig(){
  #ifdef DEBUG 
  SerialDebug("Handle request /config\n");  
  SerialDebug_printf("POST Arguments: %d\n", server->args()); 
  #endif
  StaticJsonDocument<1000> doc;
  bool isResultHtml = false;
  if (server->args() > 1){
    String s = "{";
    for (int i = 0; i < server->args();i++){
      SerialDebug_printf("%s : %s\n",server->argName(i).c_str(),server->arg(i).c_str());
      if(i>0)s+=",";
      s+= "\"";
      s+= server->argName(i);
      s+= "\":\"";
      s+= server->arg(i);
      s+= "\"";      
    }
    s+="}";    
    SerialDebug_printf("Convert post data to Json : %s\n",s.c_str());
    DeserializationError error = deserializeJson(doc, s.c_str());
      if (error) {
      #ifdef DEBUG 
      SerialDebug(F("deserializeJson() failed: "));
      SerialDebug(error.c_str());    
      #endif
      return server->send(500, "text/json", "{success:false}");
    }
    isResultHtml = true;
  }else{
    SerialDebug(server->arg("plain"));
    SerialDebug("\n");   
    DeserializationError error = deserializeJson(doc, server->arg("plain"));
      if (error) {
      #ifdef DEBUG 
      SerialDebug(F("deserializeJson() failed: "));
      SerialDebug(error.c_str());    
      #endif
      return server->send(500, "text/json", "{success:false}");
    }
    
  }

  JsonObject data = doc.as<JsonObject>();  
  strncpy(Device.wifiConfig.ssid,(const char *)data["wifi_ssid"],sizeof(Device.wifiConfig.ssid));
  strncpy(Device.wifiConfig.pass,(const char *)data["wifi_pass"],sizeof(Device.wifiConfig.pass));  
  SerialDebug_printf("wifi ssid : %s\n",Device.wifiConfig.ssid);
  SerialDebug_printf("wifi pass : %s\n",Device.wifiConfig.pass);
  Device.save();
  delay(10);
  _handleCallback(data);
  isCompleted = true;
  if(isResultHtml){
    String s = "<!DOCTYPE html>";
    s+="<html><head>";
    s+="<meta name='viewport' content='width=device-width, initial-scale=1'>";
    s+="<title>ESP8266 - Smart Device Configuration</title><body>Success</body></html>";
    server->send ( 200, "text/html", s.c_str());    
  }else{
    server->send ( 200, "text/json", "{success:true}" );    
  }
  delay(100);
}

void WebConfig::handleCallback(void (*handle) (JsonObject)){
  _handleCallback = handle;
}

void WebConfig::handleInfo(){
  SerialDebug("Handle Request : /info\n");
  SerialDebug_printf("  Config fields : %d\n",configFieldCount);
  String s = "{\"serialNo\":\"";
  s+=Device.SerialNumber();
  s+="\",fields:[";
  for(int i=0;i<configFieldCount;i++){
    SerialDebug_printf("  %s : %s\n",configFields[i].name,configFields[i].value);
    delay(100);
    if(i>0)s+=",";
    s+="{\"name\":\"";
    s+=configFields[i].name;
    s+="\",\"desc\":\"";
    s+=configFields[i].desc;
    s+="\",\"value\":\"";
    s+=configFields[i].value;
    s+="\"}";
  }
  s+="]}";
  SerialDebug_printf("Send data : %s\n",s.c_str());
  delay(100);
  server->send ( 200, "text/json", s.c_str());
}

void WebConfig::begin(bool enableCaptivePortal){
  isEnableCaptivePortal = enableCaptivePortal;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid,ap_password);
  WiFi.hostname(this->ap_ssid);
  server = new ESP8266WebServer(80);
  server->on("/", [this]{handleRoot();});
  server->on("/config",HTTP_POST, [this]{handleConfig();});
  server->on("/info", [this]{handleInfo();});
  if(isEnableCaptivePortal){
    dnsServer = new DNSServer();
    dnsServer->start(AP_DNS_PORT, "*", WiFi.softAPIP());
    server->onNotFound([this]{handleRoot();});
  }
  server->begin();
#ifdef DEBUG
  IPAddress myIP = WiFi.softAPIP();
  SerialDebug("SoftAP Address : ");
  SerialDebug(myIP);
  SerialDebug("\n");
#endif
}

WebConfig::~WebConfig(){
  server->stop(); 
}


