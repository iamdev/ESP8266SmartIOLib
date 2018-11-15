#ifndef _TASK_H
#define _TASK_H
#define MAX_TASK 32
#define TASK_MILLIS_BIT        30
#define TASK_MILLIS_BITMASK    ((~(unsigned long)0)>>(32-TASK_MILLIS_BIT))

struct task_t{
    void (*callback)(task_t&);
    long loopInterval;
    unsigned long tPrev;
    bool enabled;
};

static task_t tasks[MAX_TASK];
static int taskCount = 0;
static unsigned long task_ms_bitMask = TASK_MILLIS_BITMASK;


void createTask(void (*callback)(),int interval);
void createTask(void (*callback)(task_t&),int interval);
void runTasks();
#endif