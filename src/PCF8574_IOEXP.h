#ifndef _PCF8574_IOEXP_H
#define _PCF8574_IOEXP_H
#include <Wire.h>
#include <Arduino.h>

enum InputState{
    INPUT_STATE_NONE,
    INPUT_STATE_HOLE,
    INPUT_STATE_RELEASED,
};

struct InputStatus{
    InputState state;
    long startTime;
    long duration;
};

class PCF8574{
    public: 
        PCF8574(TwoWire *wire,int deviceAddress);
        PCF8574(int deviceAddress);         
        void begin();
        int setInputChannels(int n,const int * bits);
        int setOutputChannels(int n,const int * bits);
        int setInputInterrupt(int pin,void (*callback)(void));
        void onInputChange(void (*callback)(void));
        void inputLoop();
        void setInputActiveLevel(int state);
        void setOutputActiveLevel(int state);
        int inputActiveLevel();
        int outputActiveLevel();
        int readChannel(int ch);
        void writeChannel(int ch,int state);   
        void writeChannelToggle(int ch);     
        int getOutputState(int ch);
        InputStatus getInputStatus(int ch);
        int read(int pin);
        void write(int pin,int state);    
    protected:
        int _read();
        void _write(int data);
        int _interruptPin=-1;
        int _deviceAddress;
        int _isValid = 0;
        int _inputCount=0,_outputCount=0;
        int _inputChannels[8];        
        int _outputChannels[8];
        int _input_bitmask;
        int _output_bitmask;
        int _out_buffer,_in_buffer;
        int _inputActiveLevel = LOW,_outputActiveLevel = LOW;
        struct InputStatus _inputStatus[8];
        void (*_inputChangeCallback)(void);        
        TwoWire *_wire;
};

#endif
