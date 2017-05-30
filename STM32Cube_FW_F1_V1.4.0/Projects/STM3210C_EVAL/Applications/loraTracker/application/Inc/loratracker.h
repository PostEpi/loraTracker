#ifndef __LORATRACKER_H
#define __LORATRACKER_H

#include "stm32f1xx.h"
#include "debug.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */


/* ########################## bsp Selection ############################## */
#define BSP_DEBUG_ON

#define BSP_V500_TEST
#define BSP_HSE_EXTERNEL_12M

extern void Error_Handler(void);
#endif /* LORATRACKER */