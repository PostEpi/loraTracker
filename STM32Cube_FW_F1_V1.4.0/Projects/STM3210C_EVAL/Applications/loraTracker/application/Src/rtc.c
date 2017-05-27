#include "rtc.h"


/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle;
/* Buffer used for displaying Time */
// uint8_t aShowTime[50] = {0};
// uint8_t aShowDate[50] = {0};

static void RTC_AlarmConfig(void);


void RTC_init() 
{
    /*##-1- Configure the RTC peripheral #######################################*/
	RtcHandle.Instance = RTC;

	/* Configure RTC prescaler and RTC data registers */
	/* RTC configured as follows:
	   - Asynch Prediv  = Automatic calculation of prediv for 1 sec timebase
	   */
	RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}
	
    /*##-2- Configure Alarm ####################################################*/
	/* Configure RTC Alarm */
	RTC_AlarmConfig();
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

	DEBUG(ZONE_TRACE, ("%s %s\r\n", showdate, showtime));
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
