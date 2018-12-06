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
#include "AutoUpdate.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "Device.h"
//#define DEBUG
#include "debug.h"

struct version_t parseVersion(const char * currentVersion){
  char vstr [32];
  strncpy(vstr,currentVersion,32);  
  struct version_t v = {0,0,0};
  char * tok = strtok((char*)vstr,".");
  if(!tok)return v;
  v.major = atol(tok);
  tok = strtok(NULL,".");
  if(!tok)return v;
  v.minor = atol(tok);
  tok = strtok(NULL,".");
  if(!tok)return v;
  v.build = atol(tok);
  return v;
}

bool isNewVersion(struct version_t cv,struct version_t uv){
  if(uv.major > cv.major)return true; 
  if(uv.major < cv.major)return false; 
  if(uv.minor > cv.minor)return true;
  if(uv.minor < cv.minor)return false;
  if(uv.build > cv.build)return true;
  return false;
}

bool autoUpdate(const char*serverAddress,
                const char * currentVersion,
                const char * versionPath, 
                const char * firmwarePath,
                void(*onUpdateCallback)(const char *)){
   if(WiFi.status() == WL_CONNECTED){
      char repoVersion[32];
      char versionUrl[200];
      bool versionFound = false;
      snprintf(versionUrl,200,"%s%s",serverAddress,versionPath);
      SerialDebug_printf("Get Version from : %s\n",versionUrl);
      HTTPClient http;  //Declare an object of class HTTPClient
      http.begin(versionUrl);  //Specify request destination
      int httpCode = http.GET();                                                                  //Send the request

      if (httpCode > 0) { //Check the returning code 
        String payload = http.getString();   //Get the request response payload
        if(httpCode==200){
          strncpy(repoVersion,payload.c_str(),32);
          SerialDebug_printf("Read Version : %s\n",repoVersion);//Print the response payload
          versionFound = true;
        }
        else{
          SerialDebug_printf("Error %d:\n================\n%s\n================\n",httpCode,payload.c_str());//Print the response payload        
        }         
      }else{
        SerialDebug("Get firmware version fail!\n");
      }
  
      http.end();   //Close connection
      struct version_t cv = parseVersion(currentVersion);
      struct version_t rv = parseVersion(repoVersion);
      SerialDebug_printf("Current Version : %d.%d.%d\n",cv.major,cv.minor,cv.build);
      SerialDebug_printf("Repository Version : %d.%d.%d\n",rv.major,rv.minor,rv.build);
      if(versionFound && isNewVersion(cv,rv)){
        SerialDebug_printf("Found update version : %s to %s\n",currentVersion,repoVersion);
        if(onUpdateCallback)onUpdateCallback((const char*)repoVersion);
        char binUrl[200];
        snprintf(binUrl,200,"%s%s",serverAddress,firmwarePath);
        t_httpUpdate_return ret = ESPhttpUpdate.update(binUrl);
        switch (ret) {
          case HTTP_UPDATE_FAILED:
            SerialDebug_printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

          case HTTP_UPDATE_NO_UPDATES:
            SerialDebug_printf("HTTP_UPDATE_NO_UPDATES\n");
            break;

          case HTTP_UPDATE_OK:
            SerialDebug_printf("HTTP_UPDATE_OK\n");
            return true;
        }        
      }      
  }
  return false;
}
