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
  
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 


void Usart1_Init(u32 bound)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	/*ʹ��USART1����ʱ��*/  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	/*��λ����2*/
 	USART_DeInit(USART2);
  
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9

	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10	


  /*USART1 ��ʼ������*/
	USART_InitStructure.USART_BaudRate = bound;										//���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//����ģʽ����Ϊ�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure);										//��ʼ������2

  /*Usart1 NVIC����*/
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 
	  
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//ʹ�ܴ���2�����ж�

  USART_Cmd(USART1, ENABLE);                    			//ʹ�ܴ��� 
	USART_ClearFlag(USART1, USART_FLAG_TC);					//���������ɱ�־
}






void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
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
		/* ��������жϱ�־*/ 
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif
} 

	
void Usart1_SendString(char* s)
{
	while(*s)//����ַ���������
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
		USART_SendData(USART1 ,*s++);//���͵�ǰ�ַ�
	}
}







void Usart2_Init(u32 bound)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*ʹ��USART2����ʱ��*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	/*��λ����2*/
	USART_DeInit(USART2);  

	/*USART2_GPIO��ʼ������*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			//USART2_TXD(PA.2)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_RXD(PA.10)


	/*USART2 ��ʼ������*/
	USART_InitStructure.USART_BaudRate = bound;										//���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//����ģʽ����Ϊ�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure);										//��ʼ������2

	/*Usart1 NVIC����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//ʹ�ܴ���2�����ж�

	USART_Cmd(USART2, ENABLE);                    			//ʹ�ܴ��� 
	USART_ClearFlag(USART2, USART_FLAG_TC);					//���������ɱ�־
}


void USART2_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART2);//(USART1->DR);	//��ȡ���յ�������
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
        /* ��������жϱ�־*/ 
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
}






void Usart2_SendString(char* s)
{
	while(*s)//����ַ���������
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
		USART_SendData(USART2 ,*s++);//���͵�ǰ�ַ�
	}
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
}








void Usart3_Init(u32 bound)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*ʹ��USART3����ʱ��*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	/*��λ����3*/
	USART_DeInit(USART3);  

	/*USART3_GPIO��ʼ������*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			//USART3_TXD(PB.10)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//USART3_RXD(PB.11)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_RXD(PA.10)


	/*USART3 ��ʼ������*/
	USART_InitStructure.USART_BaudRate = bound;										//���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//����ģʽ����Ϊ�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure);										//��ʼ������3

	/*Usart1 NVIC����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//ʹ�ܴ���3�����ж�

	USART_Cmd(USART3, ENABLE);                    			//ʹ�ܴ��� 
	USART_ClearFlag(USART3, USART_FLAG_TC);					//���������ɱ�־
}


void USART3_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�
	{
//////////////////////////////////////////////////////////////////////////////////////
		Res =USART_ReceiveData(USART3);//(USART1->DR);	//��ȡ���յ�������
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
        /* ��������жϱ�־*/ 
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
}






void Usart3_SendString(char* s)
{
	while(*s)//����ַ���������
	{
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET); 
		USART_SendData(USART3 ,*s++);//���͵�ǰ�ַ�
	}
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
}













