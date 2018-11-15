#ifndef _PCA9539_IOEXP_H
#define _PCA9539_IOEXP_H
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

class PCA9539{
    public: 
        PCA9539(TwoWire *wire,int deviceAddress);
        PCA9539(int deviceAddress);         
        void begin();
        int setInputChannels(int n,const int * bits);
        int setOutputChannels(int n,const int * bits);
        int setInputInterrupt(int pin,void (*callback)(void));
        void onInputChange(void (*callback)(void));
        void inputLoop();
        int read(int ch);
        uint16_t read();
        void write(int ch,int state);  
        void write(uint16_t data);   
        void toggle(int ch);     
        int getOutputState(int ch);
        InputStatus getInputStatus(int ch);
    protected:
        uint16_t _read();
        void _write(uint16_t data);
        void _setMode(uint16_t mode);
        int _interruptPin=-1;
        int _deviceAddress;
        int _isValid = 0;
        int _inputCount=0,_outputCount=0;
        int _inputChannels[16];        
        int _outputChannels[16];
        int _input_bitmask;
        int _output_bitmask;
        int _out_buffer,_in_buffer;
        struct InputStatus _inputStatus[16];
        void (*_inputChangeCallback)(void);        
        TwoWire *_wire;
};

#endif
