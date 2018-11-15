#ifndef _SERIALCOMMAND_H
#define _SERIALCOMMAND_H
#include "HardwareSerial.h"

class SerialCommand{
  public:
    SerialCommand();
    SerialCommand(void*serial);
    void registerCommand(char*cmd,void (*callback)(const char * arg));
    void read(void);
  private:
    void* _serial;
    int cmdCount = 0;
    int maxCmd = 32;
    struct SerialCommandCallback{
      const char * cmd;
      void (*callback)(const char * arg);
    };
    SerialCommandCallback * commandList;
    char arg_buf[256];
    char cmd_buf[32];
    char*arg_ptr;
    char*cmd_ptr;
    bool has_cmd = false;    
};


#endif
