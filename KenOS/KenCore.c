/*******************************************************************************
  * @author  Ken
  * @version V1.0.0
  * @date    06-05-2020
*******************************************************************************/

#include "KenCore.h"
#include "usart.h"

INT32U OSRdyTbl;	/* 就绪表, 记录了各任务的就绪与否状态, 0-挂起, 1-就绪*/
INT32U OSNextTaskPrio;	/* 就绪表中高优先级的任务的对应位, 临时变量*/
INT32U OSPrioHighRdy;	/* 就绪表中高优先级的任务的对应位*/
INT32U OSPrioCur;		/* 当前运行任务对应位*/
INT32U OSTime;	/* 系统计数变量*/
INT32U OSIntNesting;				/*任务嵌套数*/
INT32U OSInterruptSum;				/*进入中断次数*/
INT8U OSRunning;

struct TaskCtrBlock TCB[OS_TASKS - 1]; /* 定义任务控制块 */ 
struct TaskCtrBlock *p_OSTCBCur ; /* 指向当前任务控制块的指针 */ 
struct TaskCtrBlock *p_OSTCBHighRdy ; /* 指向下一个就绪任务控制块的指针 */ 


/*在就绪表中登记任务*/
void OSSetPrioRdy(INT8U prio)
{
	OSRdyTbl |= 1 << prio;
}
 
/*在就绪表中删除任务*/
void OSDelPrioRdy(INT8U prio)
{
	OSRdyTbl &= ~(1<<prio);
}
 
/*在就绪表中查找更高级的就绪任务*/
void OSGetHighRdy(void)
{
	INT8U OSNextTaskPrio = 0;		/*任务优先级*/
 
	for (OSNextTaskPrio = 0;  \
		(OSNextTaskPrio < OS_TASKS) && (!(OSRdyTbl&(0x01<<OSNextTaskPrio))); \
		OSNextTaskPrio++ );
	OSPrioHighRdy = OSNextTaskPrio;	
}

void OSTimeDly(INT32U ticks)
{
	if( ticks > 0 ) /* 当延时有效 */
	{
		OS_ENTER_CRITICAL();
		OSDelPrioRdy(OSPrioCur); /* 把任务从就绪表中删去 */
		TCB[OSPrioCur].OSTCBDly = ticks; /* 设置任务延时节拍数 */
		OS_EXIT_CRITICAL();
		OSSched(); /* 重新调度 */
	}
} 

void TickInterrupt(void)
{
	static INT8U i;
	OSTime++;

	for(i = 0; i < OS_TASKS; i++)
	{
		if(TCB[i].OSTCBDly)
		{
			TCB[i].OSTCBDly--;
			if(TCB[i].OSTCBDly == 0) /* 当任务时钟到时,必须是由定时器减时的才行*/
			{
				OSSetPrioRdy(i); /* 使任务可以重新运行 */
			}
		}
	}
} 

void OSSched (void)
{
	OSGetHighRdy();					//找出任务就绪表中优先级最高的任务
	if(OSPrioHighRdy!=OSPrioCur)	//如果不是当前运行任务，进行任务调度
	{
		p_OSTCBCur = &TCB[OSPrioCur];				//汇编中引用			
		p_OSTCBHighRdy = &TCB[OSPrioHighRdy];		//汇编中引用
		OSPrioCur = OSPrioHighRdy;	//更新OSPrio
		OSCtxSw();					//调度任务
	}
} 

void OSTaskCreate(void  (*Task)(void  *parg), void *parg, INT32U *p_Stack, INT8U TaskID)
{
	INT32U *p_tmp = &p_Stack[31];
	if(TaskID <= OS_TASKS)
	{
		*(p_tmp) = (INT32U)0x01000000L;	/*  xPSR                        */ 
	    *(--p_tmp) = (INT32U)Task;					/*  Entry Point of the task  任务入口地址   */
	    *(--p_tmp) = (INT32U)0xFFFFFFFEL;				/*  R14 (LR)  (init value will  */

	    *(--p_tmp) = (INT32U)0x12121212L;				/*  R12                         */
	    *(--p_tmp) = (INT32U)0x03030303L;				/*  R3                          */
	    *(--p_tmp) = (INT32U)0x02020202L;				/*  R2                          */
	    *(--p_tmp) = (INT32U)0x01010101L;				/*  R1                          */
		*(--p_tmp) = (INT32U)parg;						/*  R0 : argument  输入参数     */

		*(--p_tmp) = (INT32U)0x11111111L;				/*  R11                         */
	    *(--p_tmp) = (INT32U)0x10101010L;				/*  R10                         */
	    *(--p_tmp) = (INT32U)0x09090909L;				/*  R9                          */
	    *(--p_tmp) = (INT32U)0x08080808L;				/*  R8                          */
	    *(--p_tmp) = (INT32U)0x07070707L;				/*  R7                          */
	    *(--p_tmp) = (INT32U)0x06060606L;				/*  R6                          */
	    *(--p_tmp) = (INT32U)0x05050505L;				/*  R5                          */
	    *(--p_tmp) = (INT32U)0x04040404L;				/*  R4                          */

		TCB[TaskID].OSTCBStkPtr = (INT32U)p_tmp;		/*保存堆栈地址*/
		TCB[TaskID].OSTCBDly = 0;						/*初始化任务延时*/
		OSSetPrioRdy(TaskID);
	}
} 


void OSTaskSuspend(INT8U prio)
{
	OS_ENTER_CRITICAL();		/*进入临界区*/
	TCB[prio].OSTCBDly = 0;
	OSDelPrioRdy(prio);			/*挂起任务*/
	OS_EXIT_CRITICAL();			/*退出临界区*/
 
	if(OSPrioCur == prio)		/*挂起的任务为当前运行的任务*/
	{
		OSSched();				/*重新调度*/
	}
}
 
void OSTaskResume(INT8U prio)
{
	OS_ENTER_CRITICAL();
	TCB[prio].OSTCBDly = 0;		/*设置任务延时时间为0*/
	OSSetPrioRdy(prio);			/*就绪任务*/
	OS_EXIT_CRITICAL();
 
	if(OSPrioCur > prio)		/*当前任务优先级小于恢复的任务优先级*/
	{
		OSSched();
	}
}


INT32U IDELTASK_STK[32];
/*系统空闲任务*/
void IdleTask(void *pdata)
{
	INT32U IdleCount = 0;
	while(1)
	{
		IdleCount++;
		OSSched();
	}
}

void OSStart(void)
{
	if(OSRunning == 0)
	{
		OSRunning = 1;
		
		OSTaskCreate(IdleTask, (void *)0, (INT32U *)&IDELTASK_STK[0], IdelTask_Prio);	//创建空闲任务
		OSGetHighRdy();					/*获得最高级的就绪任务*/
		OSPrioCur = OSPrioHighRdy;		/*获得最高优先级就绪任务ID*/
		p_OSTCBCur = &TCB[OSPrioCur];
		p_OSTCBHighRdy = &TCB[OSPrioHighRdy];
		OSStartHighRdy();
	}
}


void OSIntExit(void)
{
	OS_ENTER_CRITICAL();
	if(OSIntNesting > 0)
		OSIntNesting--;
	if(OSIntNesting == 0)
	{
		OSGetHighRdy();					/*找出任务优先级最高的就绪任务*/
		if(OSPrioHighRdy!=OSPrioCur)	/*当前任务并非优先级最高的就绪任务*/
		{
			p_OSTCBCur = &TCB[OSPrioCur];
			p_OSTCBHighRdy = &TCB[OSPrioHighRdy];
			OSPrioCur = OSPrioHighRdy;
			OSIntCtxSw();				/*中断级任务调度*/
		}
	}
	OS_EXIT_CRITICAL();
}

void OSTaskSwHook(void)
{
	
}
