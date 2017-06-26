/******************************************************************************
  * @file	 lcom.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date	 27-February-2017
  * @brief	 manages virtual com port
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include <stdarg.h>
#include "loratracker.h"
#include "lcom.h"
#include "timeServer.h"
#include "utilities.h"
#include "tiny_vsnprintf.h"
#include "stm3210c_loraTracker.h"

/* Force include of hal uart in order to inherite HAL_UART_StateTypeDef definition */
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_uart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
static UART_HandleTypeDef UartHandle;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* based on UART_HandleTypeDef */
#define LCOM_BUFFER_BYTE_SIZE   256

static struct
{
    char buffTx[LCOM_BUFFER_BYTE_SIZE];     /**< buffer to transmit */
    char buffRx[LCOM_BUFFER_BYTE_SIZE];     /**< Circular buffer of received chars */
    int rx_idx_free;      /**< 1st free index in BuffRx */
    int rx_idx_toread;    /**< next char to read in buffRx, when not rx_idx_free */
    HAL_LockTypeDef Lock; /**< Locking object */

    __IO HAL_UART_StateTypeDef gState;  /**< UART state information related to global Handle management
											and also related to Tx operations. */
    __IO HAL_UART_StateTypeDef RxState; /**< UART state information related to Rx operations. */
} uart_context;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief	Transmit uart_context.buffTx from start to len - 1
  * @param	1st index to transmit
  * @param	Last index not to transmit
  * @return Last index not transmitted
  */
static int buffer_transmit(int start, int len);

/**
  * @brief	Takes one character that has been received and save it in uart_context.buffRx
  * @param	received character
  */
static void receive(char rx);


static TimerEvent_t LoraPowerTimer;
static void OnLoraPowerTimer(void)
{
#if defined(BSP_V500_TEST)  
    HAL_GPIO_WritePin(LORA_POWER_EN_GPIO_PORT ,LORA_POWER_EN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LORA_RESET_EN_GPIO_PORT ,LORA_RESET_EN_PIN, GPIO_PIN_RESET);
#endif    
}

/* Functions Definition ------------------------------------------------------*/

void lcom_Init(void)
{

    /*## Configure the UART peripheral ######################################*/
    /* Put the UART peripheral in the Asynchronous mode (UART Mode) */
    /* UART1 configured as follow:
	   - Word Length = 8 Bits
	   - Stop Bit = One Stop bit
	   - Parity = ODD parity
	   - BaudRate = 921600 baud
	   - Hardware flow control disabled (RTS and CTS signals) */

#if defined(BSP_V500_TEST)
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	LORA_POWER_EN_CLK_ENABLE();

	GPIO_InitStruct.Pin 	  = LORA_POWER_EN_PIN;
	GPIO_InitStruct.Mode	  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull	  = GPIO_NOPULL;
	GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LORA_POWER_EN_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LORA_POWER_EN_GPIO_PORT ,LORA_POWER_EN_PIN, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin 	  = LORA_RESET_EN_PIN;
	GPIO_InitStruct.Mode	  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull	  = GPIO_NOPULL;
	GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LORA_RESET_EN_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LORA_RESET_EN_GPIO_PORT ,LORA_RESET_EN_PIN, GPIO_PIN_SET);
#endif

    UartHandle.Instance = LOR_USARTx;
    UartHandle.Init.BaudRate = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        Error_Handler();
    }

    lcom_IoInit();


	TimerInit(&LoraPowerTimer, OnLoraPowerTimer);
	TimerSetValue(&LoraPowerTimer, 2000); 
	TimerStart(&LoraPowerTimer);

    uart_context.gState = HAL_UART_STATE_READY;
    uart_context.RxState = HAL_UART_STATE_READY;
}

void lcom_DeInit(void)
{
    if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
    {
        Error_Handler();
    }
}

void lcom_Send(const char *format, ...)
{
    va_list args;
    static __IO uint16_t len = 0;
    uint16_t current_len;
    int start;
    int stop;

    va_start(args, format);

    BACKUP_PRIMASK();
    DISABLE_IRQ();
    if (len != 0)
    {
        if (len != LCOM_BUFFER_BYTE_SIZE)
        {
            current_len = len; /* use current_len instead of volatile len in below computation */
            len = current_len + tiny_vsnprintf_like(uart_context.buffTx + current_len,
                                                    LCOM_BUFFER_BYTE_SIZE - current_len, format, args);
        }
        RESTORE_PRIMASK();
        va_end(args);
        return;
    }
    else
    {
        len = tiny_vsnprintf_like(uart_context.buffTx, LCOM_BUFFER_BYTE_SIZE, format, args);
    }

    current_len = len;
    RESTORE_PRIMASK();

    start = 0;

    do
    {
        stop = buffer_transmit(start, current_len);
        {
            BACKUP_PRIMASK();
            DISABLE_IRQ();
            if (len == stop)
            {
                len = 0;
                RESTORE_PRIMASK();
            }
            else
            {
                start = stop;
                current_len = len;
                RESTORE_PRIMASK();
            }
        }
    } while (current_len != stop);

    va_end(args);
}

void lcom_BinarySend(const char *cmd, int size)
{
    int i;

    if (UartHandle.State == HAL_UART_STATE_TIMEOUT)
    {
        UartHandle.State = HAL_UART_STATE_READY;
    }

    for (i = 0; i < size; i++)
    {
        HAL_UART_Transmit(&UartHandle, (uint8_t *)&cmd[i], 1, 0xFFFF);
    }
    
}

HAL_StatusTypeDef lcom_ReceiveInit(void)
{
    if (uart_context.RxState != HAL_UART_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(&uart_context);

    uart_context.RxState = HAL_UART_STATE_BUSY_RX;

    /* Enable the UART Parity Error Interrupt */
    __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_PE);

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_ERR);

    /* Process Unlocked */
    __HAL_UNLOCK(&uart_context);

    return HAL_OK;
}

void lcom_IoInit(void)
{

    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
}

void lcom_IoDeInit(void)
{
}

FlagStatus lIsNewCharReceived(void)
{
    FlagStatus status;

    BACKUP_PRIMASK();
    DISABLE_IRQ();

    status = ((uart_context.rx_idx_toread == uart_context.rx_idx_free) ? RESET : SET);

    RESTORE_PRIMASK();
    return status;
}

uint8_t lGetNewChar(void)
{
    uint8_t NewChar;

    BACKUP_PRIMASK();
    DISABLE_IRQ();

    NewChar = uart_context.buffRx[uart_context.rx_idx_toread];
    uart_context.rx_idx_toread = (uart_context.rx_idx_toread + 1) % LCOM_BUFFER_BYTE_SIZE;

    RESTORE_PRIMASK();
    return NewChar;
}

void lcom_IRQHandler(void)
{
    /* USER CODE BEGIN USART2_IRQn 0 */
    UART_HandleTypeDef *huart = &UartHandle;
    char ch;
    uint32_t tmp_flag = 0, tmp_it_source = 0;

    tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE);
    tmp_it_source = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
    /* UART in mode Receiver ---------------------------------------------------*/
    if ((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
        receive((uint8_t)(huart->Instance->DR & (uint8_t)0x00FF));
    }

    tmp_flag = __HAL_UART_GET_FLAG(huart, UART_FLAG_ORE);
    /* UART Over-Run interrupt occurred ----------------------------------------*/
    if ((tmp_flag != RESET) && (tmp_it_source != RESET))
    {
            //HAL_UART_ErrorCallback(huart);
        DEBUG(ZONE_ERROR, ("LUART ERROR = %x\r\n", huart->ErrorCode));
        
        /* Clear all the error flag at once */
        __HAL_UART_CLEAR_PEFLAG(huart);

        /* Set the UART state ready to be able to start again the process */
    	huart->State = HAL_UART_STATE_READY;
    }

}

/* Private functions Definition ------------------------------------------------------*/

static int buffer_transmit(int start, int len)
{
    int i;

    if (UartHandle.State == HAL_UART_STATE_TIMEOUT)
    {
        UartHandle.State = HAL_UART_STATE_READY;
    }

    DEBUG(ZONE_TRACE, ("LCOM: buffer_transmit : %s\r\n", &uart_context.buffTx[start]));

    for (i = start; i < len; i++)
    {
        HAL_UART_Transmit(&UartHandle, (uint8_t *)&uart_context.buffTx[i], 1, 0xFFFF);
    }
    return len;
}

static void receive(char rx)
{
    int next_free;

    /** no need to clear the RXNE flag because it is auto cleared by reading the data*/
    uart_context.buffRx[uart_context.rx_idx_free] = rx;
    next_free = (uart_context.rx_idx_free + 1) % LCOM_BUFFER_BYTE_SIZE;
    if (next_free != uart_context.rx_idx_toread)
    {
        /* this is ok to read as there is no buffer overflow in input */
        uart_context.rx_idx_free = next_free;
    }
    else
    {
        /* force the end of a command in case of overflow so that we can process it */
        //uart_context.buffRx[uart_context.rx_idx_free] = '\r';
        uart_context.rx_idx_toread = 0;
        uart_context.rx_idx_free = 0;
        DEBUG(ZONE_ERROR , ("\r\n\r\n\r\n luart_context.buffRx buffer overflow \r\n\r\n\r\n"));
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
