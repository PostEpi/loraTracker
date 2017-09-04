#include "rtc.h"

/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle;
/* Buffer used for displaying Time */
// uint8_t aShowTime[50] = {0};
// uint8_t aShowDate[50] = {0};

#define WAKEUP_TIMER_ENABLE 0x32F2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef Input_Handle;

uint16_t tmpCCTIM_CHANNEL_4[2] = {0, 0};
__IO uint32_t uwLsiFreq = 0;

__IO uint32_t uwCaptureNumber = 0;
__IO uint32_t uwPeriodValue = 0;

static void RTC_Config(void);
static uint32_t GetLSIFrequency(void);

static void RTC_AlarmConfig(void);


void RTC_init()
{
	/*##-1- Configure the RTC peripheral #######################################*/
	RtcHandle.Instance = RTC;

/* Configure RTC prescaler and RTC data registers */
/* RTC configured as follows:
	   - Asynch Prediv  = Automatic calculation of prediv for 1 sec timebase
	   */
#if defined(BSP_V500_TEST)
	RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Configure Alarm ####################################################*/
	/* Configure RTC Alarm */
	RTC_AlarmConfig();
#else
	uwLsiFreq = GetLSIFrequency();
	RtcHandle.Init.AsynchPrediv = uwLsiFreq - 1; //RTC_AUTO_1_SECOND;
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Configure Alarm ####################################################*/
	/* Configure RTC Alarm */
	//RTC_AlarmConfig();
	RTC_Config();
#endif
}

/**
 * @brief  Display the current time.
 * @param  showtime : pointer to buffer
 * @retval None
 */
uint32_t RTC_GetDateTime(uint8_t *showtime, uint8_t *showdate)
{
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
	/* Display time Format : hh:mm:ss */
	sprintf((char *)showtime, "%02d:%02d:%02d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
	/* Display date Format : mm-dd-yy */
	sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);

	//DEBUG(ZONE_FUNCTION, ("[%s %s]\r\n", showdate, showtime));
	//DEBUG(ZONE_TRACE, ("%d \r\n",HAL_RTC_GetTimerValue(&RtcHandle)));

	return 0;
}

/**
 * @brief  Configure the current time and date.
 * @param  None
 * @retval None
 */
static void RTC_AlarmConfig(void)
{
	RTC_DateTypeDef sdatestructure;
	RTC_TimeTypeDef stimestructure;
	RTC_AlarmTypeDef salarmstructure;

	/*##-1- Configure the Date #################################################*/
	/* Set Date: Tuesday February 18th 2014 */
	sdatestructure.Year = 0x14;
	sdatestructure.Month = RTC_MONTH_FEBRUARY;
	sdatestructure.Date = 0x18;
	sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

	if (HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BCD) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Configure the Time #################################################*/
	/* Set Time: 02:20:00 */
	stimestructure.Hours = 0x02;
	stimestructure.Minutes = 0x20;
	stimestructure.Seconds = 0x00;

	if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-3- Configure the RTC Alarm peripheral #################################*/
	/* Set Alarm to 02:20:30 
	   RTC Alarm Generation: Alarm on Hours, Minutes and Seconds */
	salarmstructure.Alarm = RTC_ALARM_A;
	salarmstructure.AlarmTime.Hours = 0x02;
	salarmstructure.AlarmTime.Minutes = 0x20;
	salarmstructure.AlarmTime.Seconds = 0x30;

	if (HAL_RTC_SetAlarm_IT(&RtcHandle, &salarmstructure, RTC_FORMAT_BCD) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
}

/**
  * @brief  Configure the RTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */
static void RTC_Config(void)
{
	/*##-1- Configure the RTC peripheral #######################################*/
	/* Configure RTC prescaler and RTC data registers */
	/* RTC configured as follow:
      - Asynch Prediv  = Calculated automatically by HAL (based on LSI at 40kHz) */
	RtcHandle.Instance = RTC;
	RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;

	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Check if data stored in BackUp register1: Wakeup timer enable #######*/
	/* Read the Back Up Register 1 Data */
	if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) == WAKEUP_TIMER_ENABLE)
	{
		/* if the wakeup timer is enabled then desable it to disable the wakeup timer interrupt */
		if (HAL_RTCEx_DeactivateSecond(&RtcHandle) != HAL_OK)
		{
			/* Initialization Error */
			Error_Handler();
		}
	}

	/*##-3- Configure the RTC Wakeup peripheral #################################*/
	HAL_RTCEx_SetSecond_IT(&RtcHandle);

	/*##-4- Write 'wakeup timer enabled' tag in RTC Backup data Register 1 #######*/
	HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, WAKEUP_TIMER_ENABLE);
}

/**
  * @brief  Configures TIM5 to measure the LSI oscillator frequency. 
  * @param  None
  * @retval LSI Frequency
  */
static uint32_t GetLSIFrequency(void)
{
	TIM_IC_InitTypeDef TIMInput_Config;

	/* Configure the TIM peripheral *********************************************/
	/* Set TIMx instance */
	Input_Handle.Instance = TIM5;

	/* TIM5 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM5 TIM_CHANNEL_4.
     The Rising edge is used as active edge.
     The TIM5 CCR TIM_CHANNEL_4 is used to compute the frequency value. 
  ------------------------------------------------------------ */
	Input_Handle.Init.Prescaler = 0;
	Input_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	Input_Handle.Init.Period = 0xFFFF;
	Input_Handle.Init.ClockDivision = 0;
	if (HAL_TIM_IC_Init(&Input_Handle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/* Connect internally the TIM5 TIM_CHANNEL_4 Input Capture to the LSI clock output */
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_AFIO_REMAP_TIM5CH4_ENABLE();

	/* Configure the Input Capture of TIM_CHANNEL_4 */
	TIMInput_Config.ICPolarity = TIM_ICPOLARITY_RISING;
	TIMInput_Config.ICSelection = TIM_ICSELECTION_DIRECTTI;
	TIMInput_Config.ICPrescaler = TIM_ICPSC_DIV8;
	TIMInput_Config.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&Input_Handle, &TIMInput_Config, TIM_CHANNEL_4) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/* Start the TIM Input Capture measurement in interrupt mode */
	if (HAL_TIM_IC_Start_IT(&Input_Handle, TIM_CHANNEL_4) != HAL_OK)
	{
		Error_Handler();
	}

	/* Wait until the TIM5 get 2 LSI edges */
	//while(uwCaptureNumber != 2)
	{
	}

	/* Disable TIM5 CC1 Interrupt Request */
	HAL_TIM_IC_Stop_IT(&Input_Handle, TIM_CHANNEL_4);

	/* Deinitialize the TIM5 peripheral registers to their default reset values */
	HAL_TIM_IC_DeInit(&Input_Handle);

	return uwLsiFreq;
}

/**
  * @brief  Input Capture callback in non blocking mode 
  * @param  htim : TIM IC handle
  * @retval None
*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	/* Get the Input Capture value */
	tmpCCTIM_CHANNEL_4[uwCaptureNumber++] = HAL_TIM_ReadCapturedValue(&Input_Handle, TIM_CHANNEL_4);

	if (uwCaptureNumber >= 2)
	{
		if (tmpCCTIM_CHANNEL_4[0] > tmpCCTIM_CHANNEL_4[1])
		{
			/* Compute the period length */
			uwPeriodValue = (uint16_t)(0xFFFF - tmpCCTIM_CHANNEL_4[0] + tmpCCTIM_CHANNEL_4[1] + 1);
		}
		else
		{
			/* Compute the period length */
			uwPeriodValue = (uint16_t)(tmpCCTIM_CHANNEL_4[1] - tmpCCTIM_CHANNEL_4[0] + 1);
		}
		/* Frequency computation */
		uwLsiFreq = (uint32_t)SystemCoreClock / uwPeriodValue;
		uwLsiFreq *= 8;
	}
}

/**
  * @brief  RTC wakeup timer callback 
  * @param  htim : TIM IC handle
  * @retval None
*/
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	/* Toggle LED1 */
	//BSP_LED_Toggle(LED1);
}
