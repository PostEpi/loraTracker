#include "timtimer.h"
//#include "stm32f1xx_hal.h"
#include "timeServer.h"
#include "rtc.h"

static __IO uint32_t uwTimTick;
TIM_HandleTypeDef	 TimHandle;

static uint8_t led_red = 0;
static TimerEvent_t RedTimer;
static TimerEvent_t RtcTimer;

/* Private functions ---------------------------------------------------------*/
static void initTimer();
static void OnLEDRedTimer(void);
static void OnRTCTimer(void);


/**
 * @brief  Period elapsed callback in non blocking mode
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	uwTimTick++;
	TimerIrqHandler();
}


uint32_t TIM_GetTick(void)
{
	return uwTimTick;
}


void TIM_Init(void) 
{

	/* Prescaler declaration */
	uint32_t uwPrescalerValue = 0;

	/* Compute the prescaler value to have TIMx counter clock equal to 10000 Hz */
	uwPrescalerValue = (uint32_t)(SystemCoreClock / 10000) - 1;

	/* Set TIMx instance */
	TimHandle.Instance = TIMx;

	/* Initialize TIMx peripheral as follows:
	   + Period = 10000 - 1
	   + Prescaler = (SystemCoreClock/10000) - 1
	   + ClockDivision = 0
	   + Counter direction = Up
	   */
	TimHandle.Init.Period			 = 10-1;
	TimHandle.Init.Prescaler		 = uwPrescalerValue;
	TimHandle.Init.ClockDivision	 = 0;
	TimHandle.Init.CounterMode		 = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	/*##-2- Start the TIM Base generation in interrupt mode ####################*/
	/* Start Channel1 */
	if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
	{
		/* Starting Error */
		Error_Handler();
	}

	uwTimTick = 0;
    
    initTimer();
}

static void initTimer()
{
    TimerInit(&RedTimer, OnLEDRedTimer);
	TimerSetValue(&RedTimer, 2000); /* 1000ms */
	TimerStart(&RedTimer);

	TimerInit(&RtcTimer, OnRTCTimer);
	TimerSetValue(&RtcTimer, 1000); /* 1000ms */
	TimerStart(&RtcTimer);
}


/* Private functions ---------------------------------------------------------*/

static void OnLEDRedTimer(void)
{

	//led_red ? BSP_LED_On(LED_RED) : BSP_LED_Off(LED_RED);
	led_red = 1 - led_red;
	TimerStart(&RedTimer);
}

static void OnRTCTimer(void)
{
	uint8_t aShowTime[50] = {0};
	uint8_t aShowDate[50] = {0};

	RTC_GetDateTime(aShowTime, aShowDate);
	TimerStart(&RtcTimer);
}
