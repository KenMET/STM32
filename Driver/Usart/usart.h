#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "delay.h"
#include "string.h"
#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

#include "math.h"


#define USART_REC_LEN  			64  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����

#define Log_Uartx	Usart1_SendString	//����ϵͳ��Ϣ���ʹ��ں�


//////////////////////////////////////////////////////////////////////////////////
//��������в����ĺ궨��
#define ADD 		0
#define DELET 		1
#define SELECT 		2
#define MOVE 		3
#define EXCHANGE 	4
//////////////////////////////////////////////////////////////////////////////////

extern unsigned char Got_Usart1_Data[USART_REC_LEN];
extern unsigned char Got_Usart2_Data[USART_REC_LEN];
extern unsigned char Got_Usart3_Data[USART_REC_LEN];
extern unsigned char Usart1_Finish;
extern unsigned char Usart2_Finish;
extern unsigned char Usart3_Finish;

//////////////////////////////////////////////////////////////////////////////////	  	
extern u8  Usart1_Situation;         		//�����ַ�������ʼ�洢λ��
extern u8  Usart2_Situation;         		//�����ַ�������ʼ�洢λ��
extern u8  Usart3_Situation;         		//�����ַ�������ʼ�洢λ��

	



extern void Usart1_Init(u32 bound);
extern void Usart2_Init(u32 bound);
extern void Usart3_Init(u32 bound);
extern void Usart1_SendString(char* s);
extern void Usart2_SendString(char* s);
extern void Usart3_SendString(char* s);
extern void Clean_Str(unsigned char *Words);
extern void Send_AT_Cmd(unsigned char *cmd,unsigned char *keyword,unsigned char usartx,uint16_t delaytime);


#endif


