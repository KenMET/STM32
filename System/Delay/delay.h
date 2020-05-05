#ifndef __Delay_H_
#define __Delay_H_

#include"stm32f10x.h"


extern void Delay_Init(u8 SYSCLK);
extern void Delay_us(u32 nus); 
extern void Delay_ms(u16 nms); 

#endif






