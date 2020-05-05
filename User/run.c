#include "run.h"
#include "sys.h" 
#include "usart.h" 
#include "delay.h" 
#include "led.h" 
#include "malloc.h"


void led_task(void *p_arg)
{
	printf("led_task Create Success\n");
	while(1)
	{
		LED0(0);
		delay_ms(88);
		LED0(1);
		delay_ms(88);
		LED0(0);
		delay_ms(88);
		LED0(1);
		delay_ms(1680);
	}
}

void uart_task(void *p_arg)
{
	int cnt = 0;
	printf("uart_task Create Success\n");
	while(1)
	{
		printf("Heart Beat[%d]\n", cnt++);
		delay_ms(10000);
	}
}

void hardware_init(void)
{
	Cache_Enable();					//Open L1-Cache
	HAL_Init();						//HAL Lib Init
	Stm32_Clock_Init(160,5,2,4);	//Set Clock to 400Mhz
	delay_init(400);
	uart_init(115200);
	LED_Init();
	printf("Hardware Init Success\n");	
}

#ifdef SYSTEM_SUPPORT_OS

task_info user_task[TASK_MAX_CNT];

#define START_TASK_PRIO		3
#define START_STK_SIZE 		512
OS_TCB StartTaskTCB;
CPU_STK START_TASK_STK[START_STK_SIZE];

#define USER_START_STK_SIZE 		16384
CPU_STK user_stack[USER_START_STK_SIZE];

int get_free_task(void)
{
	int i = 0;
	for(i=0; i<TASK_MAX_CNT;i++)
		if (!user_task[i].flag)
			return i;
	return i;
}

int get_task_cnt(void)
{
	int i = 0, cnt = 0;
	for(i=0; i<TASK_MAX_CNT;i++)
		if (user_task[i].flag)
			cnt++;
	return cnt;
}

int find_free_stack(int size)
{
	int start = 0;
	int i = 0, task_cnt = 0;
	task_cnt = get_task_cnt();
	for(i=0; i<task_cnt;i++)
	{
		if(user_task[i].flag)
		{
			start += user_task[i].stack_size;
		}
	}
	if(USER_START_STK_SIZE - start >= size)
	{
		return start;
	}
	printf("No Free Space to malloc\n");
	return 0;
}

int push_into_task_list(task_fun fun, char* name, OS_PRIO prio, CPU_STK_SIZE stack_size)
{
	int task_cnt = 0;
	task_info *tmp = NULL;

	task_cnt = get_task_cnt();
	
	if (task_cnt >= TASK_MAX_CNT)
		return -1;
	
	tmp = &user_task[task_cnt];

	printf("Start Push Task[%s]\n", name);
	strcpy(tmp->name, name);
	tmp->fun = fun;
	tmp->prio = prio;
	tmp->stack_size = stack_size;
	//tmp->stack = (CPU_STK *)malloc(sizeof(CPU_STK) * stack_size);
	tmp->stack = &user_stack[find_free_stack(stack_size)];
	tmp->stack_start = 0;
	tmp->msg_qty = 0;
	tmp->opt = OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP;
	tmp->tick = 0;
	tmp->flag = 1;
	return 0;
}

void create_user_task(void)
{
	int i = 0;
	task_info *tmp = NULL;
	int task_cnt = get_task_cnt();
	printf("Task Start Address is %p\n", &user_stack[0]);
	printf("%d Need To Create...\n", task_cnt);
	for(i=0; i<task_cnt; i++)
	{
		tmp = &user_task[i];
		printf("%s Creating, stack at %p[%d]...\n", tmp->name, tmp->stack, tmp->stack_size);
		OSTaskCreate((OS_TCB 	* )&tmp->tcb,		
					(CPU_CHAR	* )tmp->name, 		
					(OS_TASK_PTR )tmp->fun, 			
					(void		* )0,					
					(OS_PRIO	  )tmp->prio,     
					(CPU_STK   * )tmp->stack,	
					(CPU_STK_SIZE)tmp->stack_size/10,	
					(CPU_STK_SIZE)tmp->stack_size,		
					(OS_MSG_QTY  )tmp->msg_qty,					
					(OS_TICK	  )tmp->tick,					
					(void   	* )0,					
					(OS_OPT      )tmp->opt,
					(OS_ERR 	* )&tmp->err);
	}
}

void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	printf("start_task Create Success\n");
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);            
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN
	OSSchedRoundRobinCfg(DEF_ENABLED,10,&err);  
#endif		
	OS_CRITICAL_ENTER();
	
	create_user_task();

	OS_CRITICAL_EXIT();			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);	 
}

void create_init_task(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	OSInit(&err);
	OS_CRITICAL_ENTER();

	/*Add your business task here*/
	push_into_task_list(led_task, "LED TASK", TASK_MAX_CNT - 1, 128);
	push_into_task_list(uart_task, "UART TASK", TASK_MAX_CNT - 2, 128);

	/*User Task End*/

	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,
				 (CPU_CHAR	* )"start task",
                 (OS_TASK_PTR )start_task,
                 (void		* )0,
                 (OS_PRIO	  )START_TASK_PRIO,
                 (CPU_STK   * )&START_TASK_STK[0],
                 (CPU_STK_SIZE)START_STK_SIZE/10,
                 (CPU_STK_SIZE)START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK	  )0,
                 (void   	* )0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR 	* )&err);
	
	OS_CRITICAL_EXIT();
	OSStart(&err);
}
#endif



void run(void)
{
	while(1)
	{
		LED0(0);
		delay_ms(80);
		LED0(1);
		delay_ms(80);
		LED0(0);
		delay_ms(80);
		LED0(1);
		delay_ms(2000);
	}
}

