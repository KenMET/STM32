#include "usart.h"

u8 Got_Usart1_Data[USART_REC_LEN];
u8 Got_Usart2_Data[USART_REC_LEN];
u8 Got_Usart3_Data[USART_REC_LEN];

u8 Usart1_Situation=0;
u8 Usart2_Situation=0;
u8 Usart3_Situation=0;

u8 Usart1_Finish = 0;
u8 Usart2_Finish = 0;
u8 Usart3_Finish = 0;
  
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 


void Usart1_Init(u32 bound)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	/*使能USART1外设时钟*/  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	/*复位串口2*/
 	USART_DeInit(USART2);
  
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9

	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10	


  /*USART1 初始化设置*/
	USART_InitStructure.USART_BaudRate = bound;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
  USART_Init(USART1, &USART_InitStructure);										//初始化串口2

  /*Usart1 NVIC配置*/
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//从优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 
	  
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//使能串口2接收中断

  USART_Cmd(USART1, ENABLE);                    			//使能串口 
	USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志
}






void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		if(Res == '@')
			Usart1_Situation = 0;	
		Got_Usart1_Data[Usart1_Situation++] = Res;
		Got_Usart1_Data[Usart1_Situation]	= '\0';
//		if(Res == '~')
//		{
//			Got_Usart1_Data[Usart1_Situation]	= '\0';
		 	Usart1_Finish = 1;
//		}
						
//////////////////////////////////////////////////////////////////////////////////////
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) ; 
	}
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	{ 
		/* 清楚接收中断标志*/ 
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
} 

	
void Usart1_SendString(char* s)
{
	while(*s)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
		USART_SendData(USART1 ,*s++);//发送当前字符
	}
}







void Usart2_Init(u32 bound)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*使能USART2外设时钟*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	/*复位串口2*/
	USART_DeInit(USART2);  

	/*USART2_GPIO初始化设置*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			//USART2_TXD(PA.2)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//调用库函数中的GPIO初始化函数，初始化USART1_RXD(PA.10)


	/*USART2 初始化设置*/
	USART_InitStructure.USART_BaudRate = bound;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
	USART_Init(USART2, &USART_InitStructure);										//初始化串口2

	/*Usart1 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//从优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//使能串口2接收中断

	USART_Cmd(USART2, ENABLE);                    			//使能串口 
	USART_ClearFlag(USART2, USART_FLAG_TC);					//清除发送完成标志
}


void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据
		if(Res == '@')
			Usart2_Situation = 0;	
		Got_Usart2_Data[Usart2_Situation++] = Res;
		Got_Usart2_Data[Usart2_Situation]	= '\0';
//		if(Res == '~')
//		{
//			Got_Usart1_Data[Usart1_Situation]	= '\0';
		 	Usart2_Finish = 1;
//		}
			
//////////////////////////////////////////////////////////////////////////////////////
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) ; 
	}
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
    { 
        /* 清楚接收中断标志*/ 
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
}






void Usart2_SendString(char* s)
{
	while(*s)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
		USART_SendData(USART2 ,*s++);//发送当前字符
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
}








void Usart3_Init(u32 bound)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*使能USART3外设时钟*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	/*复位串口3*/
	USART_DeInit(USART3);  

	/*USART3_GPIO初始化设置*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			//USART3_TXD(PB.10)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//USART3_RXD(PB.11)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//调用库函数中的GPIO初始化函数，初始化USART1_RXD(PA.10)


	/*USART3 初始化设置*/
	USART_InitStructure.USART_BaudRate = bound;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
	USART_Init(USART3, &USART_InitStructure);										//初始化串口3

	/*Usart1 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//从优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//使能串口3接收中断

	USART_Cmd(USART3, ENABLE);                    			//使能串口 
	USART_ClearFlag(USART3, USART_FLAG_TC);					//清除发送完成标志
}


void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART3);//(USART1->DR);	//读取接收到的数据
		if(Res == '@')
			Usart3_Situation = 0;			
		Got_Usart3_Data[Usart3_Situation++] = Res;
		Got_Usart3_Data[Usart3_Situation]	= '\0';
//		if(Res == '~')
//		{
//			Got_Usart3_Data[Usart3_Situation]	= '\0';
			Usart3_Finish = 1;		
//		}
			
//////////////////////////////////////////////////////////////////////////////////////
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) ; 
	}
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
    { 
        /* 清楚接收中断标志*/ 
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
}






void Usart3_SendString(char* s)
{
	while(*s)//检测字符串结束符
	{
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET); 
		USART_SendData(USART3 ,*s++);//发送当前字符
	}
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
}













