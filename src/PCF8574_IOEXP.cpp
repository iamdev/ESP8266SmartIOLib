#include "PCF8574_IOEXP.h"
#include <Arduino.h>
#define PCF8574_I2C_ALTADDR 0x18

#define DEBUG
#include "debug.h"

#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(pin) (pin)
#endif

PCF8574::PCF8574(TwoWire *wire,int deviceAddress)
    :_wire(wire),_deviceAddress(deviceAddress)
{
    _wire->beginTransmission(_deviceAddress);
    _isValid = (_wire->endTransmission()==0);
}

PCF8574::PCF8574(int deviceAddress)
    :PCF8574(&Wire,deviceAddress)
{

}

void PCF8574::begin(){
    int addr = _deviceAddress;
    _wire->beginTransmission(addr);
    int error = _wire->endTransmission();
    SerialDebug_printf("Detect PCF8574 ...");
    if(error!=0){
        addr = _deviceAddress|PCF8574_I2C_ALTADDR;
        _wire->beginTransmission(addr);
        error = _wire->endTransmission();   
    }

    if(error==0){
        _deviceAddress = addr;
        SerialDebug_printf("Address %2X\n",addr);
        this->_isValid = 1;
    }else{
      SerialDebug("NOT FOUND!!\n"); 
    }

    if(_isValid){
        int data =_read(); 
        _in_buffer = (data|_output_bitmask)&0xFFFF;    
        _out_buffer = (data|_input_bitmask)&0xFFFF;    
    }
}

int PCF8574::setInputChannels(int n,const int * bits){
    _inputCount = n;
    _input_bitmask = 0;

    for(int i=0;i<n;i++){
        _inputChannels[i] = bits[i];
        _input_bitmask |= 1<<bits[i];
    }    
}

int PCF8574::setOutputChannels(int n,const int * bits){
    _outputCount = n;
    _output_bitmask = 0;
    for(int i=0;i<n;i++){
        _outputChannels[i] = bits[i];
        _output_bitmask |= 1<<bits[i];
    }    
}

int PCF8574::setInputInterrupt(int pin,void (*callback)(void)){
    pinMode(pin,INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), callback, FALLING); 
}


void PCF8574::setInputActiveLevel(int state){
    this->_inputActiveLevel = state;
}
void PCF8574::setOutputActiveLevel(int state){
    this->_outputActiveLevel = state;
}


int PCF8574::inputActiveLevel(){
    return this->_inputActiveLevel;
}
int PCF8574::outputActiveLevel(){
    return this->_outputActiveLevel;
}

int PCF8574::_read(){
    _wire->beginTransmission( _deviceAddress );
    _wire->endTransmission();
    _wire->requestFrom(_deviceAddress, (int)1 ); // request only one byte
    unsigned long t=millis();
    int data = 0xFFFF;
    while ( millis() < t+1000 && _wire->available() == 0 ); // waiting 
    if(_wire->available()){
        data = _wire->read();
    }
    _wire->endTransmission();  
    return data;
}

void PCF8574::_write(int data){
    _wire->beginTransmission(_deviceAddress);
    _wire->write(data&0xFF);
    _wire->endTransmission(); 
}

int PCF8574::read(int pin){
    SerialDebug("IO_EXP :: read data ");
    SerialDebug(_read(),BIN);
    SerialDebug("\n");
    return (_read()>>pin)&0x1;
}

void PCF8574::write(int pin,int state){
    SerialDebug_printf("IO_EXP :: Write bit %d to %d\n",pin,state);
    SerialDebug("-----------------------------------\n");

    SerialDebug("  IO_EXP :: Output buffer =");
    SerialDebug(_out_buffer&0xFF,BIN);
    SerialDebug("\n");

    _out_buffer &= (~(1<<pin))&0xFFFF;
    _out_buffer |= (state?1:0)<<pin;

    int data = _out_buffer|_input_bitmask;
    SerialDebug("  IO_EXP :: Write data =");
    SerialDebug(data,BIN);
    SerialDebug("\n");
    SerialDebug("-----------------------------------\n");    
    _write(data);
}

int PCF8574::readChannel(int ch){
    if(ch<0 && ch>= _inputCount)return 0;
    int out = read(_inputChannels[ch]);
    if(_inputActiveLevel == LOW) out = !out;
    return out;
}

void PCF8574::writeChannel(int ch,int state){
    if(ch<0 && ch>= _outputCount)return;
    if(_outputActiveLevel == LOW) state = !state;
    this->write(_outputChannels[ch],state);
}

void PCF8574::writeChannelToggle(int ch){
    if(ch<0 && ch>= _outputCount)return;
    int state = getOutputState(ch);
    if(_outputActiveLevel == LOW) state = !state;    
    this->write(_outputChannels[ch],state);
}

int PCF8574::getOutputState(int ch){
    if(ch<0 && ch>= _outputCount)return 0;
    int state = _out_buffer>>_outputChannels[ch];
    return state&1;
}

InputStatus PCF8574::getInputStatus(int ch){
    if(ch>=0 && ch<_outputCount){
        return _inputStatus[ch];
    }
}


void PCF8574::onInputChange(void (*callback)(void)){
    _inputChangeCallback = callback;
}
void PCF8574::inputLoop(){
    if(!_isValid)return;
    int in = _read()|_output_bitmask;    
    in &=0xFFFF;
    if(in!= _in_buffer){
        for(int i=0;i<_inputCount;i++){
            int p = _inputChannels[i];
            int p1 = (_in_buffer>>p)&1;
            int p2 = (in>>p)&1;
            if(_inputActiveLevel==HIGH){
                p1 = !p1;
                p2 = !p2;
            }
            if(p1 && !p2){
                _inputStatus[i].startTime = millis();
                _inputStatus[i].state = INPUT_STATE_HOLE;
                _inputStatus[i].duration = 0;
            }else if (!p1 && p2){
                _inputStatus[i].state = INPUT_STATE_RELEASED;
                _inputStatus[i].duration = millis() - _inputStatus[i].startTime;
            }else if(p1 && p2){
                _inputStatus[i].state = INPUT_STATE_NONE;
            }
        }
        _in_buffer = in;
        if(_inputChangeCallback)_inputChangeCallback();
    }
}