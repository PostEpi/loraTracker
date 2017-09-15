/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/main.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   This example shows how to retarget the C library printf function
  *          to the UART.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "vdb.h"
#include "wisol.h"
#include <string.h>

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup UART_Printf
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* enable debug zone */
int DebugFlag = 0x3;//0xf;
bool loracmdbypass = false;

/* UART handler declaration */
UART_HandleTypeDef UartHandle;



/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
void SystemClock_Config(void);
void Error_Handler(void);

#define USER_PROCESS_CHAR_BUFFER_SIZE   100
#define DEBUG_RESPONSE_MAX          2
#define DEBUG_RESPONSE_LEN          10
static const uint8_t DEBUG_RESPONSE[DEBUG_RESPONSE_MAX][DEBUG_RESPONSE_LEN] = 
{
    "$$WARA$$",
    "$$LORA$$",
};
static const int debugmode[10] = {0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff};


#ifdef USE_DEBUGLOG_DRVIER
/* Private functions ---------------------------------------------------------*/
static void USER_Init(void)
{
    debuglog_Init();
    debuglog_ReceiveInit();
}

static void USER_Process() 
{
	char ch;
    bool bfound = false;
	static int msgcount = 0;
    static char buffer[USER_PROCESS_CHAR_BUFFER_SIZE];

    while (IsdebugReceived() == SET)
    {
        ch = getdebugChar();

		//DEBUG(ZONE_FUNCTION, ("%c", ch));

        buffer[msgcount++] = ch;
#ifdef DEMD_IOT_SK_PREPARE_SPEC
        if( ch == '\n')
        {
#else
        // if you send commands in a typical serial program,
        // there is no line feed value(\r). 
        if(ch == '\r' || ch == '\n')
        {
            // To add LF to buffer.
            buffer[msgcount++] = '\n';
#endif            
            bfound = false;
            int i, cmdLength;
            for(i = 0; i < DEBUG_RESPONSE_MAX; i++)
            {
                cmdLength = strlen((const char *)DEBUG_RESPONSE[i]);
                if(!strncmp((const char*)buffer, (const char*)DEBUG_RESPONSE[i], cmdLength))
                {
                    bfound = true;
                    switch(i)
                    {
                    case 0:
                        if(buffer[cmdLength] >= '0' && buffer[cmdLength] <= '9')
                        {
                            DebugFlag = debugmode[buffer[cmdLength] - '0'];
                        }
                        break;
                    case 1:
                        // if(buffer[cmdLength] == '0' || buffer[cmdLength] == '1')
                        // {
                        //     if(buffer[cmdLength] == '0')  DebugFlag = 0;
                        //     else DebugFlag = 1<<9;
                        // }
                        BSP_Lora_Wakeup();
                        LBPRINTF(&buffer[cmdLength], msgcount-cmdLength);
                        break;
                    }
                }
            }

#ifdef DEMD_IOT_SK_PREPARE_SPEC            
            // it's passed to the lora;
            if(bfound == false) 
            {
				// You must be able to send commands directly to the lora chip for test certification. Sk spec 
                DEBUG(ZONE_TRACE, ("USER_Process : it has been sent to lora(%s)", buffer));
                //updateDB(LOR, buffer, msgcount, CID_TX_BYPASS_PROCESS_FOR_SKIOT);
                //int changetime = 1;
                //DEMD_IOcontrol(DEMD_REPORT_PERIOD_CHANGE, &changetime, 1, NULL, 0);
                BSP_Lora_Wakeup();
                LBPRINTF(buffer, msgcount);
            }
#else
            // it's passed to the lora;
            if(bfound == false && loracmdbypass) 
            {
				// You must be able to send commands directly to the lora chip for test certification. Sk spec 
                DEBUG(ZONE_TRACE, ("USER_Process : bypass (%s)", buffer));
                //updateDB(LOR, buffer, msgcount, CID_TX_BYPASS_PROCESS_FOR_SKIOT);
                //int changetime = 1;
                //DEMD_IOcontrol(DEMD_REPORT_PERIOD_CHANGE, &changetime, 1, NULL, 0);
                BSP_Lora_Wakeup();
                LBPRINTF(buffer, msgcount);
            }
#endif
            memset(buffer, 0, USER_PROCESS_CHAR_BUFFER_SIZE);
            msgcount = 0; 
        }

        if(msgcount > USER_PROCESS_CHAR_BUFFER_SIZE)
        { 
            memset(buffer, 0, USER_PROCESS_CHAR_BUFFER_SIZE);
            msgcount = 0;
        }
    }

}

#else
static void USER_Process() 
{
	char ch;
    bool bfound = false;
	static int msgcount = 0;
    static char buffer[USER_PROCESS_CHAR_BUFFER_SIZE];

    if (HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 0x10) != HAL_TIMEOUT)
    {

		DEBUG(ZONE_FUNCTION, ("%c", ch));

        buffer[msgcount++] = ch;
        if(ch == '\n')
        {
            bfound = false;
            int i, cmdLength;
            for(i = 0; i < DEBUG_RESPONSE_MAX; i++)
            {
                cmdLength = strlen((const char *)DEBUG_RESPONSE[i]);
                if(!strncmp((const char*)buffer, (const char*)DEBUG_RESPONSE[i], cmdLength))
                {
                    bfound = true;
                    switch(i)
                    {
                    case 0:
                        if(buffer[cmdLength] >= '0' && buffer[cmdLength] <= '9')
                        {
                            DebugFlag = debugmode[buffer[cmdLength] - '0'];
                        }
                        break;
                    case 1:
                        DEBUG(ZONE_TRACE, (" it has been sent to lora(%s)", buffer[cmdLength]));
                        //updateDB(LOR, buffer, msgcount, 0xff);
                        LBPRINTF(&buffer[cmdLength], msgcount-cmdLength);

                        // if(buffer[cmdLength] == '0' || buffer[cmdLength] == '1')
                        // {
                        //     if(buffer[cmdLength] == '0')  DebugFlag = 0;
                        //     else DebugFlag = 1<<9;
                        // }
                        break;
                    }
                }
            }
#ifdef DEMD_IOT_SK_PREPARE_SPEC            
            // it's passed to the lora;
            if(bfound == false) 
            {
                DEBUG(ZONE_TRACE, ("DONTCAREGPS : it has been sent to lora(%s)", buffer));
                //updateDB(LOR, buffer, msgcount, 0xff);
                LBPRINTF(buffer, msgcount);
            }
#endif
            memset(buffer, 0, USER_PROCESS_CHAR_BUFFER_SIZE);
            msgcount = 0; 
        }

        if(msgcount > USER_PROCESS_CHAR_BUFFER_SIZE)
        { 
            memset(buffer, 0, USER_PROCESS_CHAR_BUFFER_SIZE);
            msgcount = 0;
        }
    }

}
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* STM32F107xC HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
    HAL_Init();

    /* Configure the system clock to 72 MH5z */
    SystemClock_Config();

    /* Initialize BSP Led for LED_RED */
    BSP_LED_Init(LED_RED);
    BSP_LED_Init(LED_GREEN);

    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_GREEN);

#if defined(BSP_V500_TEST)
    BSP_OUTGPIO_init(OUTPUT_WDI, GPIO_PIN_RESET);
    BSP_OUTGPIO_init(OUTPUT_PS_HOLD, GPIO_PIN_SET);
    BSP_OUTGPIO_init(OUTPUT_WDE, GPIO_PIN_SET);
    BSP_OUTGPIO_init(OUTPUT_GPS_POWER, GPIO_PIN_RESET);
    BSP_OUTGPIO_init(OUTPUT_LORA_POWER, GPIO_PIN_RESET);
    BSP_OUTGPIO_init(OUTPUT_LORA_RESET, GPIO_PIN_SET);
#else
    BSP_OUTGPIO_init(OUTPUT_WKUP, GPIO_PIN_SET);
    HAL_Delay(1);
    BSP_OUTGPIO_Low(OUTPUT_WKUP);
  
    BSP_OUTGPIO_init(OUTPUT_NRST, GPIO_PIN_SET);
    BSP_OUTGPIO_init(OUTPUT_PRST, GPIO_PIN_RESET);

    // The flow bwtween the system and lora must be syncronized. 
    BSP_Lora_HW_Reset();

    /* Initialize BSP input pin */
    BSP_Input_Init(INPUT_FACTORY, INPUT_MODE_GPIO);
    if(BSP_Input_GetState(INPUT_FACTORY) == 1)
    {
        loracmdbypass = true;
        DebugFlag = 0xf;
    }
    
    DebugFlag = 0xf;

#endif

#ifndef USE_DEBUGLOG_DRVIER
    /*##-1- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART configured as follows:
      - Word Length = 8 Bits (7 data bit + 1 parity bit) : BE CAREFUL : Program 7 data bits + 1 parity bit in PC HyperTerminal
      - Stop Bit    = One Stop bit
      - Parity      = ODD parity
      - BaudRate    = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals) */
    UartHandle.Instance = LOG_USARTx;

    UartHandle.Init.BaudRate = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();

    }
#else
    USER_Init();
#endif
    /* Output a message on Hyperterminal using printf function */
    printf("\n\r *** Lora Board Start ver.%d***\n\r\n\r", BSP_GetAppVersion() );

    if(loracmdbypass)
    {
        printf("\n\r *** factory mode is enabled***\n\r\n\r");
    }

    RTC_init();
    TIM_Init();
    
    DEMD_Init();
    ECMD_Init();
    LCMD_Init();
    GCMD_Init();

    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_GREEN);



    // while(1)
    // {
    //     if(BSP_Input_GetState(INPUT_FACTORY))
    //     {
    //         printf("\n\r *** High ***\n\r\n\r");
    //     }
    //     else 
    //     {
    //         printf("\n\r *** Low ***\n\r\n\r");
    //     }
    // }
#if 0
    int status = 0, sizeofstatus = 1;
    DEMD_IOcontrol(DEMD_REPORT_GET_ACTIVATION_STATUS, NULL, 0, &status, &sizeofstatus);
    loracmdbypass = (status == 1)?true:false;
#endif

    /* Infinite loop */
    while (1)
    {
        USER_Process();
        ECMD_Process();
        LCMD_Process();
        GCMD_Process();
        DEMD_Process();
    }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
#ifndef USE_DEBUGLOG_DRVIER

    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART2 and Loop until the end of transmission */
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
#else 
    DLPRINTF((const char *)&ch);    
#endif

    return ch;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 25000000
  *            HSE PREDIV1                    = 5
  *            HSE PREDIV2                    = 5
  *            PLL2MUL                        = 8
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};

/* Configure PLLs ------------------------------------------------------*/
/* PLL2 configuration: PLL2CLK = (HSE / HSEPrediv2Value) * PLL2MUL = (25 / 5) * 8 = 40 MHz */
/* PREDIV1 configuration: PREDIV1CLK = PLL2CLK / HSEPredivValue = 40 / 5 = 8 MHz */
/* PLL configuration: PLLCLK = PREDIV1CLK * PLLMUL = 8 * 9 = 72 MHz */

/* Enable HSE Oscillator and activate PLL with HSE as source */

#if defined(BSP_V500_TEST)
    oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL6;

#if 0
    oscinitstruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
    oscinitstruct.PLL2.PLL2State = RCC_PLL2_OFF;
    oscinitstruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
    oscinitstruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
#endif

#else
    oscinitstruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState = RCC_HSE_ON;
    oscinitstruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
    oscinitstruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
    oscinitstruct.PLL.PLLState = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL = RCC_PLL_MUL9;
    oscinitstruct.PLL2.PLL2State = RCC_PLL2_ON;
    oscinitstruct.PLL2.PLL2MUL = RCC_PLL2_MUL10;
    oscinitstruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV2;
#endif

    if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)
    {
        /* Initialization Error */
        while (1)
            ;
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
    clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2) != HAL_OK)
    {
        /* Initialization Error */
        while (1)
            ;
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    /* Turn LED_RED on */
    BSP_LED_On(LED_RED);
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,*/
    printf("Wrong parameters value: file %s on line %d\r\n", file, line); 

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
