#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "KenCore.h"

INT32U USART_TASK_STK[128];
void UsartTask(void *pdata)
{
	while(1)
	{
		Log_Uartx("UsartTask Running...\n");
		OSTimeDly(100);
	}
}

INT32U TEST1_TASK_STK[128];
void Test1Task(void *pdata)
{
	while(1)
	{
		Log_Uartx("Test1Task Running...\n");
		OSTimeDly(200);
	}
}

int main(void)
{	
	Delay_Init(72);
	Usart1_Init(115200);
	Log_Uartx("Uart1 Activate Succeed...\n");
	TIM3_Int_Init(99, 7199, TickInterrupt);		// 溢出时间(us) = 10 * 1000 us
	Log_Uartx("Timer3 Activate Succeed...\n");

	OSTaskCreate(UsartTask, (void *)0, (INT32U *)&USART_TASK_STK[0], 10);
	OSTaskCreate(Test1Task, (void *)0, (INT32U *)&TEST1_TASK_STK[0], 11);
	Log_Uartx("UsartTask Create Succeed...\n");

	Log_Uartx("KenOS Starting...\n");
	OSStart();
	
	
	Log_Uartx("System Activate Succeed...\n");
	while (1)
	{
			//Log_Uartx("System Running...\n");
			Delay_ms(5000);
	}
}









