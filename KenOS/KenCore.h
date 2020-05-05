/*******************************************************************************
  * @author  Ken
  * @version V1.0.0
  * @date    06-05-2020
*******************************************************************************/


#ifndef __OS_CORE_H
#define __OS_CORE_H


#define OS_TASKS 32
#define IdelTask_Prio 31		      /*空闲任务优先级*/
#define OS_TICKS_PER_SEC 100      /* 设置一秒内的时钟节拍数*/ 

typedef unsigned char   BOOLEAN;
typedef unsigned char   INT8U;    /* Unsigned 8 bit quantity */
typedef signed char     INT8S;    /* Signed 8 bit quantity*/
typedef unsigned int    INT16U;   /* Unsigned 16 bit quantity */
typedef signed int      INT16S;   /* Signed 16 bit quantity*/
typedef unsigned long   INT32U;   /* Unsigned 32 bit quantity */
typedef signed long     INT32S;   /* Signed 32 bit quantity */ 

struct TaskCtrBlock /* 任务控制块数据结构 */
{
  INT32U OSTCBStkPtr; /* 保存任务的堆栈顶 */
  INT32U OSTCBDly; /* 任务延时时钟 */
};

extern INT32U OSRdyTbl;	/* 就绪表, 记录了各任务的就绪与否状态, 0-挂起, 1-就绪*/
extern INT32U OSNextTaskPrio;	/* 就绪表中高优先级的任务的对应位, 临时变量*/
extern INT32U OSPrioHighRdy;	/* 就绪表中高优先级的任务的对应位*/
extern INT32U OSPrioCur;		/* 当前运行任务对应位*/
extern INT32U OSTime;	/* 系统计数变量*/
extern INT32U OSIntNesting;				/*任务嵌套数*/
extern INT32U OSInterruptSum;				/*进入中断次数*/
extern INT8U OSRunning;
extern struct TaskCtrBlock *p_OSTCBCur ; /* 指向当前任务控制块的指针 */ 
extern struct TaskCtrBlock *p_OSTCBHighRdy ; /* 指向下一个就绪任务控制块的指针 */ 

void OSTimeDly(INT32U ticks);		/*设置任务延时时间*/
void TickInterrupt(void);		/*定时器中断对任务延时处理函数*/
void IdleTask(void *pdata);		/*系统空闲任务*/
void OSSched(void);				/*任务切换*/
void OSStart(void);				/*多任务系统开始*/
void OSIntExit(void);			/*中断退出函数*/
 
void OSTaskCreate(void  (*Task)(void  *parg), void *parg, INT32U *p_Stack, INT8U TaskID);	/*创建任务函数*/
void OSTaskSuspend(INT8U prio);	/*挂起指定任务*/
void OSTaskResume(INT8U prio);		/*回复指定的挂起任务*/
void OSTaskSwHook(void);		/*空函数*/

/*in asm function*/
void OS_EXIT_CRITICAL(void);	/*退出临界区*/
void OS_ENTER_CRITICAL(void);	/*进入临界区*/
void OSStartHighRdy(void);		/*调度第一个任务*/
void OSCtxSw(void);				/*函数级任务切换*/
void OSIntCtxSw(void);			/*中断级任务切换*/


#endif /* __OS_CORE_H */
/******************* (C) COPYRIGHT 2020 STMicroelectronics *****END OF FILE****/
