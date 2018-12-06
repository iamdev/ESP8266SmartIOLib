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

#include "PCA9539_IOEXP.h"
#include <Arduino.h>

//#define DEBUG
#include "debug.h"

#define PCA9539_INPUT_REGISTER       0x00
#define PCA9539_OUTPUT_REGISTER      0x02
#define PCA9539_INVERS_REGISTER      0x04
#define PCA9539_CONFIG_REGISTER      0x06

#ifndef digitalPinToInterrupt
#define digitalPinToInterrupt(pin) (pin)
#endif

#ifdef DEBUG
void print_binary(int num,int nbit){
    int m = 1<<(nbit-1);
    SerialDebug_printf("%04X [",num);
    for(int i = 0;i<nbit;i++){        
        SerialDebug((num & m)?"1":"0");
        if((i+1)%4==0)SerialDebug(" ");
        m>>=1;
    }
    SerialDebug_printf("]");
}
#else
#define print_binary(num,nbit)
#endif

PCA9539::PCA9539(TwoWire *wire,int deviceAddress)
    :_wire(wire),_deviceAddress(deviceAddress)
{
    _wire->beginTransmission(_deviceAddress);
    _isValid = (_wire->endTransmission()==0);    
    _output_bitmask = 0;
    _input_bitmask = 0;

}

PCA9539::PCA9539(int deviceAddress)
    :PCA9539(&Wire,deviceAddress)
{
}

void PCA9539::_setMode(uint16_t mode){
    _wire->beginTransmission(_deviceAddress);
    _wire->write(PCA9539_CONFIG_REGISTER);
    _wire->write(mode&0xFF);
    _wire->write((mode>>8)&0xFF);
    _wire->endTransmission();
}
void PCA9539::begin(){
    int addr = _deviceAddress;
    _wire->beginTransmission(addr);
    int error = _wire->endTransmission();
    SerialDebug_printf("Detect PCA9539 ...");
    if(error==0){
        _deviceAddress = addr;
        SerialDebug_printf("Address %2X\n",addr);
        this->_isValid = 1;
    }else{
      SerialDebug("NOT FOUND!!\n"); 
    }

    if(_isValid){
        //int data =_read(); 
        //_in_buffer = (data|_output_bitmask)&0xFFFF;    
        //_out_buffer = (data|_input_bitmask)&0xFFFF; 
        _setMode(~_output_bitmask);
        //write(0xFFFF);

        int in = _read()|_output_bitmask;    
        in &=0xFFFF;
        _in_buffer = in;
    }
}

int PCA9539::setInputChannels(int n,const int * bits){
    _inputCount = n;
    _input_bitmask = 0;

    for(int i=0;i<n;i++){
        _inputChannels[i] = bits[i];
        _input_bitmask |= 1<<bits[i];
    }    
    SerialDebug("Input bit masks : ");
    print_binary(_input_bitmask,16);
    SerialDebug("\n");
    _setMode(~_output_bitmask); 
}

int PCA9539::setOutputChannels(int n,const int * bits){
    _outputCount = n;
    _output_bitmask = 0;
    for(int i=0;i<n;i++){
        _outputChannels[i] = bits[i];
        _output_bitmask |= 1<<bits[i];    
    }
    _input_bitmask = (~_output_bitmask)&0xFFFF;    
    SerialDebug("Output bit masks : ");
    print_binary(_output_bitmask,16);
    SerialDebug("\n");
    if(_isValid){
        _setMode(~_output_bitmask);   
        this->write(0xFFFF);
    }   
}

int PCA9539::setInputInterrupt(int pin,void (*callback)(void)){
    pinMode(pin,INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), callback, FALLING); 
}

uint16_t PCA9539::_read(){    
    _wire->beginTransmission( _deviceAddress );
    _wire->write(PCA9539_INPUT_REGISTER);
    _wire->endTransmission();
    _wire->requestFrom(_deviceAddress, (int)2 ); // request only one byte
    unsigned long t=millis();
    int data = 0xFFFF;
    while ( millis() < t+1000 && _wire->available() == 0 ); // waiting 
    if(_wire->available()){
        data = _wire->read();
        data |= (_wire->read()&0xFF)<<8;
    }
    _wire->endTransmission();  
    return data;
}

void PCA9539::_write(uint16_t data){    
    byte b[] = {PCA9539_OUTPUT_REGISTER,data&0xFF,(data>>8)&0xFF};
    SerialDebug_printf("PCA9539 : Write Data   => %02X,%02X,%02X\n",b[0],b[1],b[2]);
    delay(10);
    _wire->beginTransmission(_deviceAddress);
    _wire->write(b[0]);
    _wire->write(b[1]);
    _wire->write(b[2]);
    int n = _wire->endTransmission();     
    if(n!=0){
        SerialDebug_printf("Send data error %d\n",n);    
    }
}

uint16_t PCA9539::read(){
    int data = this->_read();
    SerialDebug("PCA9539 :: Read => ");
    print_binary(data,16);
    SerialDebug("\n");
    data &= _input_bitmask;
    int out = 0;
    for(int i=0;i<_inputCount;i++){
        int b = _inputChannels[i];
        out <<=1;        
        out |= ((data>>b)&1)?1:0;
    }
    SerialDebug("PCA9539 :: Out Channels => ");
    SerialDebug(out,BIN);
    SerialDebug("\n");
    return out;
}

void PCA9539::write(uint16_t data){    
    for(int i=0;i<_outputCount;i++){
        int pin = _outputChannels[i];
        _out_buffer &= (~(1<<pin))&0xFFFF;
        _out_buffer |= ((data&(1<<i))?1:0)<<pin;
    }
    this->_write(_out_buffer|_input_bitmask);
}

int PCA9539::read(int ch){
    if(ch<0 && ch>= _inputCount)return 0;
    int pin = _inputChannels[ch];
    SerialDebug("PCA9539 :: read data ");
    int data = _read();
    print_binary(data,16);
    SerialDebug("\n");
    int out = (data>>pin)&0x1;
    return !out;
}

void PCA9539::write(int ch,int state){
    if(ch<0 && ch>= _outputCount)return;
    int pin = _outputChannels[ch];
    #ifdef DEBUG
    SerialDebug_printf("PCA9539 :: Write bit %d to %d\n",pin,state);
    SerialDebug("-----------------------------------\n");

    SerialDebug("  PCA9539 :: Output buffer =");
    print_binary(_out_buffer,16);
    SerialDebug("\n");
    #endif

    _out_buffer &= (~(1<<pin))&0xFFFF;
    _out_buffer |= (state?1:0)<<pin;
    int data = _out_buffer|_input_bitmask;
    #ifdef DEBUG
    SerialDebug("  PCA9539 :: Write data =");
    print_binary(data,16);
    SerialDebug("\n");
    SerialDebug("-----------------------------------\n");    
    #endif
    _write(data);
}

int PCA9539::toggle(int ch){
    if(ch<0 && ch>= _outputCount)return 0;
    int state = !getOutputState(ch);
    this->write(ch,state);
    return state;
}

int PCA9539::getOutputState(int ch){
    if(ch<0 && ch>= _outputCount)return 0;
    int state = _out_buffer>>_outputChannels[ch];
    return state&1;
}

InputStatus PCA9539::getInputStatus(int ch){
    if(ch>=0 && ch<_outputCount){
        return _inputStatus[ch];
    }
}


void PCA9539::onInputChange(void (*callback)(void)){
    _inputChangeCallback = callback;
}
void PCA9539::inputLoop(){
    if(!_isValid)return;
    int in = _read()|_output_bitmask;    
    in &=0xFFFF;
    if(in!= _in_buffer){
        for(int i=0;i<_inputCount;i++){
            int p = _inputChannels[i];
            int p1 = (_in_buffer>>p)&1;
            int p2 = (in>>p)&1;
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
