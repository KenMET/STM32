#ifndef _RUN_H
#define _RUN_H

typedef void (*task_fun)(void *);

#ifdef SYSTEM_SUPPORT_OS
#include "includes.h"

#define TASK_MAX_CNT    16


typedef struct{
    unsigned char flag;
    OS_TCB tcb;
    char name[32];
    task_fun fun;
    OS_PRIO prio;
    CPU_STK * stack;
    CPU_STK_SIZE stack_size;
    int stack_start;
    OS_MSG_QTY msg_qty;				
    OS_TICK	 tick;								
    OS_OPT opt;
    OS_ERR err;
}task_info;


void create_init_task(void);
int push_into_task_list(task_fun fun, char* name, OS_PRIO prio, CPU_STK_SIZE stack_size);
#endif


void run(void);
void hardware_init(void);


#endif
