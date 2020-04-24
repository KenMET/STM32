#include "sys.h" 
#include "usart.h" 
#include "delay.h" 
#include "led.h" 

int main(void)
{
	Cache_Enable();					//��L1-Cache
	HAL_Init();						//��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);	//����ʱ��,400Mhz
	delay_init(400);				//��ʱ��ʼ��
	uart_init(115200);				//���ڳ�ʼ��
	LED_Init();						//��ʼ��LEDʱ��  
	while(1)
	{
		LED0(0);					//DS0��
		delay_ms(80);
		LED0(1);					//DS0��
		delay_ms(80);
		LED0(0);					//DS0��
		delay_ms(80);
		LED0(1);					//DS0��
		delay_ms(2000);
	}
}
