#include "delay.h"

static u8  fac_us=0;//us��ʱ������
static u16 fac_ms=0;//ms��ʱ������

/*��ʱ����ʼ������*/  
//SYSCLKΪϵͳʱ��Ƶ�ʳ���1000000���ֵ���������ϵͳʱ��Ƶ��Ϊ72MHz������øú���ʱ�˴���ʵ��Ϊ72  
void Delay_Init(u8 SYSCLK) 
{  
    SysTick->CTRL&=0xFFFFFFFB;  
    fac_us=SYSCLK/8;  
    fac_ms=(u16)(fac_us*1000);       
}  
  
//��ʱnms
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//��72M������,nms<=1864
void Delay_ms(u16 nms)
{        
	 u32 temp; 
	 SysTick->LOAD=(u32)nms*fac_ms;      // ����������ʱ��������װ�ص�ֵ */
	 SysTick->VAL =0x00;                 // VAL�Ĵ����д���ŵ�ǰ��������ֵ  */
	 SysTick->CTRL=0x01 ;                // Systick��ʱ����ʹ��λ   ��ʼ��ʱ */
	 do
	 {
	  	temp=SysTick->CTRL;
	 }	 
	 while(temp&0x01&&!(temp&(1<<16)));   //���ȼ�鶨ʱ��ʹ��λ�Ƿ�Ϊ1 
	                                      //Systick��ʱ�����������  λ16����1 ����ȡ���λ��0
	                                        //  �ȴ�ʱ�䵽���ʱ���ر�  */ 
	 SysTick->CTRL=0x00;       //�رռ�����
	 SysTick->VAL =0X00;       //��ռ�����       
}


  
//��ʱnus
//nusΪҪ��ʱ��us��.
void Delay_us(u32 nus)
{  
	 u32 temp;
	 SysTick->LOAD=nus*fac_us; //ʱ�����     
	 SysTick->VAL=0x00;        //��ռ�����
	 SysTick->CTRL=0x01 ;      //��ʼ����  
	 do
	 {
	  	temp=SysTick->CTRL;
	 }
	 while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��  
	 SysTick->CTRL=0x00;       //�رռ�����
	 SysTick->VAL =0X00;       //��ռ����� 
 
}




