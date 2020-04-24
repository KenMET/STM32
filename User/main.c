#include "sys.h" 
#include "usart.h" 
#include "delay.h" 
#include "led.h" 

int main(void)
{
	Cache_Enable();					//打开L1-Cache
	HAL_Init();						//初始化HAL库
	Stm32_Clock_Init(160,5,2,4);	//设置时钟,400Mhz
	delay_init(400);				//延时初始化
	uart_init(115200);				//串口初始化
	LED_Init();						//初始化LED时钟  
	while(1)
	{
		LED0(0);					//DS0亮
		delay_ms(80);
		LED0(1);					//DS0灭
		delay_ms(80);
		LED0(0);					//DS0亮
		delay_ms(80);
		LED0(1);					//DS0灭
		delay_ms(2000);
	}
}
