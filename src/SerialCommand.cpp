#include "SerialCommand.h";
#include <Arduino.h>

SerialCommand::SerialCommand()
  :SerialCommand(&Serial)
{
}

SerialCommand::SerialCommand(void* serial)
:  _serial(serial)
{
    commandList = NULL;
    cmdCount = 0;
    cmd_ptr = cmd_buf;
    arg_ptr = arg_buf;

}

void SerialCommand::registerCommand(char*cmd,void (*callback)(const char * arg))
{
    int c = cmdCount++;
    commandList =  (SerialCommandCallback *) realloc(commandList, (c + 1) * sizeof(SerialCommandCallback));
    commandList[c].cmd = (const char*)cmd;
    commandList[c].callback = callback;
}

void SerialCommand::read(void){  
  while(((Stream*)_serial)->available()){
    char c = ((Stream*)_serial)->read(); 
    if(!has_cmd){
      if(cmd_ptr>cmd_buf && (c==' ' || c=='\n' ||c==':')){
        has_cmd = true; 
      }else{
        *cmd_ptr++ = c;
        *cmd_ptr = 0;
      } 
    }else if(c>=32){
      *arg_ptr++ = c;
      *arg_ptr = 0;
    }
    if(c=='\n'){
        for(int i = 0;i<cmdCount;i++){
          if(strcmp(commandList[i].cmd,cmd_buf)==0){
            commandList[i].callback(String(arg_buf).c_str());            
            break;
          }
        }        
        has_cmd = false;      
        arg_ptr = arg_buf;
        cmd_ptr = cmd_buf;
    }
  }
}  