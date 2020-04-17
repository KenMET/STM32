#include "delay.h"
#include "usart.h"





int main(void)
{	
	Delay_Init(72);
	Usart1_Init(115200);
	Log_Uartx("Uart1 Activate Succeed...\n");

	Log_Uartx("System Activate Succeed...\n");
	while (1)
	{
			Log_Uartx("System Running...\n");
			Delay_ms(1000);
	}
}









