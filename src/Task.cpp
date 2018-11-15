#include "Task.h"
#include <Arduino.h>
#define DEBUG
#include "debug.h"

void createTask(void (*callback)(task_t &),int interval)
{
     struct task_t ts = {callback,interval,0,true}; 
     tasks[taskCount++] = ts;   
} 

void runTasks(){    
    unsigned long t = millis()&task_ms_bitMask;    
    for(int i=0;i<taskCount;i++){
        if(!tasks[i].enabled)continue;
        unsigned long next = (tasks[i].tPrev + tasks[i].loopInterval)&task_ms_bitMask;        
        if(t>=next){
            tasks[i].tPrev = t;
            tasks[i].callback(tasks[i]);
        }
    }   
}