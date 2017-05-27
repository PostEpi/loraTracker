#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm3210c_loratracker.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Exported functions ------------------------------------------------------- */

void RTC_Init(); 
uint32_t RTC_GetDateTime(uint8_t *showtime, uint8_t *showdate);
#endif /* __RTC_H */