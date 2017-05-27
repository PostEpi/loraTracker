#ifndef __TIMER_H
#define __TIMER_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm3210c_loratracker.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Exported functions ------------------------------------------------------- */

void TIM_Init(); 
uint32_t TIM_GetTick();
#endif /* __TIMER_H */