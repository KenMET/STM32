#ifndef __TIMER_H_
#define __TIMER_H_


#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "misc.h"

typedef void (*timer_fun)(void);

extern void TIM3_Int_Init(u16 arr, u16 psc, timer_fun tmp);

#endif
