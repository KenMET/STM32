#include "delay.h"

static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数

/*定时器初始化函数*/  
//SYSCLK为系统时钟频率除以1000000后的值，例：如果系统时钟频率为72MHz，则调用该函数时此处的实参为72  
void Delay_Init(u8 SYSCLK) 
{  
    SysTick->CTRL&=0xFFFFFFFB;  
    fac_us=SYSCLK/8;  
    fac_ms=(u16)(fac_us*1000);       
}  
  
//延时nms
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//对72M条件下,nms<=1864
void Delay_ms(u16 nms)
{        
	 u32 temp; 
	 SysTick->LOAD=(u32)nms*fac_ms;      // 当倒数至零时，将被重装载的值 */
	 SysTick->VAL =0x00;                 // VAL寄存器中存放着当前倒计数的值  */
	 SysTick->CTRL=0x01 ;                // Systick定时器的使能位   开始计时 */
	 do
	 {
	  	temp=SysTick->CTRL;
	 }	 
	 while(temp&0x01&&!(temp&(1<<16)));   //首先检查定时器使能位是否为1 
	                                      //Systick定时器倒数至零后  位16被置1 被读取后该位置0
	                                        //  等待时间到达或定时器关闭  */ 
	 SysTick->CTRL=0x00;       //关闭计数器
	 SysTick->VAL =0X00;       //清空计数器       
}


  
//延时nus
//nus为要延时的us数.
void Delay_us(u32 nus)
{  
	 u32 temp;
	 SysTick->LOAD=nus*fac_us; //时间加载     
	 SysTick->VAL=0x00;        //清空计数器
	 SysTick->CTRL=0x01 ;      //开始倒数  
	 do
	 {
	  	temp=SysTick->CTRL;
	 }
	 while(temp&0x01&&!(temp&(1<<16)));//等待时间到达  
	 SysTick->CTRL=0x00;       //关闭计数器
	 SysTick->VAL =0X00;       //清空计数器 
 
}




